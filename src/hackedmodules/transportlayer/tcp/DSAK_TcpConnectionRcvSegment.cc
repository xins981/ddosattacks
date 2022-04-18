#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"

#include "DSAK_TcpConnection.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {

        Define_Module(DSAK_TcpConnection);

        TcpEventCode DSAK_TcpConnection::processSegmentInSynSent(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr)
        {
            EV_DETAIL << "Processing segment in SYN_SENT\n";

                //"
                // first check the ACK bit
                //
                //   If the ACK bit is set
                //
                //     If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset (unless
                //     the RST bit is set, if so drop the segment and return)
                //
                //       <SEQ=SEG.ACK><CTL=RST>
                //
                //     and discard the segment.  Return.
                //
                //     If SND.UNA =< SEG.ACK =< SND.NXT then the ACK is acceptable.
                //"
                if (tcpseg->getAckBit()) {
                    if (seqLE(tcpseg->getAckNo(), state->iss) || seqGreater(tcpseg->getAckNo(), state->snd_nxt)) {
                        if (tcpseg->getRstBit())
                            EV_DETAIL << "ACK+RST bit set but wrong AckNo, ignored\n";
                        else {
                            EV_DETAIL << "ACK bit set but wrong AckNo, sending RST\n";
                            sendRst(tcpseg->getAckNo(), destAddr, srcAddr, tcpseg->getDestPort(), tcpseg->getSrcPort());
                        }
                        return TCP_E_IGNORE;
                    }

                    EV_DETAIL << "ACK bit set, AckNo acceptable\n";
                }

                //"
                // second check the RST bit
                //
                //   If the RST bit is set
                //
                //     If the ACK was acceptable then signal the user "error:
                //     connection reset", drop the segment, enter CLOSED state,
                //     delete TCB, and return.  Otherwise (no ACK) drop the segment
                //     and return.
                //"
                if (tcpseg->getRstBit()) {
                    if (tcpseg->getAckBit()) {
                        EV_DETAIL << "RST+ACK: performing connection reset\n";
                        sendIndicationToApp(TCP_I_CONNECTION_RESET);

                        return TCP_E_RCV_RST;
                    }
                    else {
                        EV_DETAIL << "RST without ACK: dropping segment\n";

                        return TCP_E_IGNORE;
                    }
                }

                //"
                // third check the security and precedence -- not done
                //
                // fourth check the SYN bit
                //
                //   This step should be reached only if the ACK is ok, or there is
                //   no ACK, and it the segment did not contain a RST.
                //
                //   If the SYN bit is on and the security/compartment and precedence
                //   are acceptable then,
                //"
                if (tcpseg->getSynBit()) {
                    //
                    //   RCV.NXT is set to SEG.SEQ+1, IRS is set to
                    //   SEG.SEQ.  SND.UNA should be advanced to equal SEG.ACK (if there
                    //   is an ACK), and any segments on the retransmission queue which
                    //   are thereby acknowledged should be removed.
                    //
                    state->rcv_nxt = tcpseg->getSequenceNo() + 1;
                    state->rcv_adv = state->rcv_nxt + state->rcv_wnd;

                    emit(rcvAdvSignal, state->rcv_adv);

                    state->irs = tcpseg->getSequenceNo();
                    receiveQueue->init(state->rcv_nxt);

                    if (tcpseg->getAckBit()) {
                        state->snd_una = tcpseg->getAckNo();
                        sendQueue->discardUpTo(state->snd_una);

                        if (state->sack_enabled)
                            rexmitQueue->discardUpTo(state->snd_una);

                        // although not mentioned in RFC 793, seems like we have to pick up
                        // initial snd_wnd from the segment here.
                        updateWndInfo(tcpseg, true);
                    }

                    // this also seems to be a good time to learn our local IP address
                    // (was probably unspecified at connection open)
                    tcpMain->updateSockPair(this, destAddr, srcAddr, tcpseg->getDestPort(), tcpseg->getSrcPort());

                    //"
                    //   If SND.UNA > ISS (our SYN has been ACKed), change the connection
                    //   state to ESTABLISHED, form an ACK segment
                    //
                    //     <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
                    //
                    //   and send it.  Data or controls which were queued for
                    //   transmission may be included.  If there are other controls or
                    //   text in the segment then continue processing at the sixth step
                    //   below where the URG bit is checked, otherwise return.
                    //"
                    if (seqGreater(state->snd_una, state->iss)) {
                        EV_INFO << "SYN+ACK bits set, connection established.\n";

                        // RFC says "continue processing at the sixth step below where
                        // the URG bit is checked". Those steps deal with: URG, segment text
                        // (and PSH), and FIN.
                        // Now: URG and PSH we don't support yet; in SYN+FIN we ignore FIN;
                        // with segment text we just take it easy and put it in the receiveQueue
                        // -- we'll forward it to the user when more data arrives.
                        if (tcpseg->getFinBit())
                            EV_DETAIL << "SYN+ACK+FIN received: ignoring FIN\n";

                        if (B(packet->getByteLength()) > tcpseg->getHeaderLength()) {
                            updateRcvQueueVars();

                            if (hasEnoughSpaceForSegmentInReceiveQueue(packet, tcpseg)) {    // enough freeRcvBuffer in rcvQueue for new segment?
                                receiveQueue->insertBytesFromSegment(packet, tcpseg);    // TBD forward to app, etc.
                            }
                            else {    // not enough freeRcvBuffer in rcvQueue for new segment
                                state->tcpRcvQueueDrops++;    // update current number of tcp receive queue drops

                                emit(tcpRcvQueueDropsSignal, state->tcpRcvQueueDrops);

                                EV_WARN << "RcvQueueBuffer has run out, dropping segment\n";
                                return TCP_E_IGNORE;
                            }
                        }

                        if (tcpseg->getUrgBit() || tcpseg->getPshBit())
                            EV_DETAIL << "Ignoring URG and PSH bits in SYN+ACK\n"; // TBD

                        if (tcpseg->getHeaderLength() > TCP_MIN_HEADER_LENGTH) // Header options present?
                            readHeaderOptions(tcpseg);

                        return TCP_E_IGNORE;
                        // notify tcpAlgorithm (it has to send ACK of SYN) and app layer
                        state->ack_now = true;
                        tcpAlgorithm->established(true);
                        tcpMain->emit(Tcp::tcpConnectionAddedSignal, this);
                        sendEstabIndicationToApp();

                        //ECN
                        if (state->ecnSynSent) {
                            if (tcpseg->getEceBit() && !tcpseg->getCwrBit()) {
                                state->ect = true;
                                EV << "ECN-setup SYN-ACK packet was received... ECN is enabled.\n";
                            } else {
                                state->ect = false;
                                EV << "non-ECN-setup SYN-ACK packet was received... ECN is disabled.\n";
                            }
                            state->ecnSynSent = false;
                        } else {
                            state->ect = false;
                            if (tcpseg->getEceBit() && !tcpseg->getCwrBit())
                                EV << "ECN-setup SYN-ACK packet was received... ECN is disabled.\n";
                        }

                        // This will trigger transition to ESTABLISHED. Timers and notifying
                        // app will be taken care of in stateEntered().
                        return TCP_E_RCV_SYN_ACK;
                    }

                    //"
                    //   Otherwise enter SYN-RECEIVED, form a SYN,ACK segment
                    //
                    //     <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
                    //
                    //   and send it.  If there are other controls or text in the
                    //   segment, queue them for processing after the ESTABLISHED state
                    //   has been reached, return.
                    //"
                    EV_INFO << "SYN bit set: sending SYN+ACK\n";
                    state->snd_max = state->snd_nxt = state->iss;
                    sendSynAck();
                    startSynRexmitTimer();

                    // Note: code below is similar to processing SYN in LISTEN.

                    // For consistency with that code, we ignore SYN+FIN here
                    if (tcpseg->getFinBit())
                        EV_DETAIL << "SYN+FIN received: ignoring FIN\n";

                    // We don't send text in SYN or SYN+ACK, but accept it. Otherwise
                    // there isn't much left to do: RST, SYN, ACK, FIN got processed already,
                    // so there's only URG and PSH left to handle.
                    if (B(packet->getByteLength()) > tcpseg->getHeaderLength()) {
                        updateRcvQueueVars();

                        if (hasEnoughSpaceForSegmentInReceiveQueue(packet, tcpseg)) {    // enough freeRcvBuffer in rcvQueue for new segment?
                            receiveQueue->insertBytesFromSegment(packet, tcpseg);    // TBD forward to app, etc.
                        }
                        else {    // not enough freeRcvBuffer in rcvQueue for new segment
                            state->tcpRcvQueueDrops++;    // update current number of tcp receive queue drops

                            emit(tcpRcvQueueDropsSignal, state->tcpRcvQueueDrops);

                            EV_WARN << "RcvQueueBuffer has run out, dropping segment\n";
                            return TCP_E_IGNORE;
                        }
                    }

                    if (tcpseg->getUrgBit() || tcpseg->getPshBit())
                        EV_DETAIL << "Ignoring URG and PSH bits in SYN\n"; // TBD

                    return TCP_E_RCV_SYN;
                }

                //"
                // fifth, if neither of the SYN or RST bits is set then drop the
                // segment and return.
                //"
                return TCP_E_IGNORE;
        }

    }
}


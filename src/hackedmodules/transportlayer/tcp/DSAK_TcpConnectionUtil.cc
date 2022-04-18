
#include <string.h>
#include <algorithm>    // min,max

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"

#include "DSAK_TcpConnection.h"

namespace dsak
{
    namespace tcp
    {
        void DSAK_TcpConnection::initConnection(TcpOpenCommand *openCmd)
        {
            // create send queue
            sendQueue = tcpMain->createSendQueue();
            sendQueue->setConnection(this);

            // create receive queue
            receiveQueue = tcpMain->createReceiveQueue();
            receiveQueue->setConnection(this);

            // create SACK retransmit queue
            rexmitQueue = new TcpSackRexmitQueue();
            rexmitQueue->setConnection(this);

            // create algorithm
            const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();

            if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
                tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");

            tcpAlgorithm = check_and_cast<TcpAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass,"inet::tcp::TcpConnection"));
            tcpAlgorithm->setConnection(this);

            // create state block
            state = tcpAlgorithm->getStateVariables();
            configureStateVariables();
            tcpAlgorithm->initialize();
        }

        void DSAK_TcpConnection::configureStateVariables()
        {
            long advertisedWindowPar = tcpMain->par("advertisedWindow");
            state->ws_support = tcpMain->par("windowScalingSupport");    // if set, this means that current host supports WS (RFC 1323)
            state->ws_manual_scale = tcpMain->par("windowScalingFactor"); // scaling factor (set manually) to help for Tcp validation
            state->ecnWillingness = tcpMain->par("ecnWillingness"); // if set, current host is willing to use ECN
            if (!state->ws_support && (advertisedWindowPar > TCP_MAX_WIN || advertisedWindowPar <= 0))
                throw cRuntimeError("Invalid advertisedWindow parameter: %ld", advertisedWindowPar);

            state->rcv_wnd = advertisedWindowPar;
            state->rcv_adv = advertisedWindowPar;

            if (state->ws_support && advertisedWindowPar > TCP_MAX_WIN) {
                state->rcv_wnd = TCP_MAX_WIN;    // we cannot to guarantee that the other end is also supporting the Window Scale (header option) (RFC 1322)
                state->rcv_adv = TCP_MAX_WIN;    // therefore TCP_MAX_WIN is used as initial value for rcv_wnd and rcv_adv
            }

            state->maxRcvBuffer = advertisedWindowPar;
            state->delayed_acks_enabled = tcpMain->par("delayedAcksEnabled");    // delayed ACK algorithm (RFC 1122) enabled/disabled
            state->nagle_enabled = tcpMain->par("nagleEnabled");    // Nagle's algorithm (RFC 896) enabled/disabled
            state->limited_transmit_enabled = tcpMain->par("limitedTransmitEnabled");    // Limited Transmit algorithm (RFC 3042) enabled/disabled
            state->increased_IW_enabled = tcpMain->par("increasedIWEnabled");    // Increased Initial Window (RFC 3390) enabled/disabled
            state->snd_mss = tcpMain->par("mss");    // Maximum Segment Size (RFC 793)
            state->ts_support = tcpMain->par("timestampSupport");    // if set, this means that current host supports TS (RFC 1323)
            state->sack_support = tcpMain->par("sackSupport");    // if set, this means that current host supports SACK (RFC 2018, 2883, 3517)

            if (state->sack_support) {
                std::string algorithmName1 = "TcpReno";
                std::string algorithmName2 = tcpMain->par("tcpAlgorithmClass");

                if (algorithmName1 != algorithmName2) {    // TODO add additional checks for new SACK supporting algorithms here once they are implemented
                    EV_DEBUG << "If you want to use TCP SACK please set tcpAlgorithmClass to TcpReno\n";

                    ASSERT(false);
                }
            }
        }

        void DSAK_TcpConnection::sendToIP(Packet *packet, const Ptr<TcpHeader>& tcpseg)
        {
            // record seq (only if we do send data) and ackno
            if (packet->getByteLength() > B(tcpseg->getChunkLength()).get())
                emit(sndNxtSignal, tcpseg->getSequenceNo());

            emit(sndAckSignal, tcpseg->getAckNo());

            // final touches on the segment before sending
            tcpseg->setSrcPort(localPort);
            tcpseg->setDestPort(remotePort);
            ASSERT(tcpseg->getHeaderLength() >= TCP_MIN_HEADER_LENGTH);
            ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_LENGTH);
            ASSERT(tcpseg->getChunkLength() == tcpseg->getHeaderLength());
            state->sentBytes = packet->getByteLength();    // resetting sentBytes to 0 if sending a segment without data (e.g. ACK)

            EV_INFO << "Sending: ";
            printSegmentBrief(packet, tcpseg);

            // TBD reuse next function for sending

            IL3AddressType *addressType = remoteAddr.getAddressType();
            packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

            if (ttl != -1 && packet->findTag<HopLimitReq>() == nullptr)
                packet->addTag<HopLimitReq>()->setHopLimit(ttl);

            if (dscp != -1 && packet->findTag<DscpReq>() == nullptr)
                packet->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

            if (tos != -1 && packet->findTag<TosReq>() == nullptr)
                packet->addTag<TosReq>()->setTos(tos);

            auto addresses = packet->addTagIfAbsent<L3AddressReq>();
            addresses->setSrcAddress(localAddr);
            addresses->setDestAddress(remoteAddr);

            // ECN:
            // We decided to use ECT(1) to indicate ECN capable transport.
            //
            // rfc-3168, page 6:
            // Routers treat the ECT(0) and ECT(1) codepoints
            // as equivalent.  Senders are free to use either the ECT(0) or the
            // ECT(1) codepoint to indicate ECT.
            //
            // rfc-3168, page 20:
            // For the current generation of TCP congestion control algorithms, pure
            // acknowledgement packets (e.g., packets that do not contain any
            // accompanying data) MUST be sent with the not-ECT codepoint.
            //
            // rfc-3168, page 20:
            // ECN-capable TCP implementations MUST NOT set either ECT codepoint
            // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
            packet->addTagIfAbsent<EcnReq>()->setExplicitCongestionNotification((state->ect && !state->sndAck && !state->rexmit) ? IP_ECN_ECT_1 : IP_ECN_NOT_ECT);

            tcpseg->setCrc(0);
            tcpseg->setCrcMode(tcpMain->crcMode);

            insertTransportProtocolHeader(packet, Protocol::tcp, tcpseg);

            tcpMain->sendFromConn(packet, "ipOut");
        }

    } // namespace dsak
} // namespace inet


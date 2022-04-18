
#include "DSAK_TcpConnection.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {
        Define_Module(DSAK_TcpConnection);

        void DSAK_TcpConnection::initConnection(DSAK_Tcp *_mod, int _socketId)
        {
            Enter_Method_Silent();

            tcpMain = _mod;
            socketId = _socketId;

            fsm.setName(getName());
            fsm.setState(TCP_S_INIT);

            // queues and algorithm will be created on active or passive open

            the2MSLTimer = new cMessage("2MSL");
            connEstabTimer = new cMessage("CONN-ESTAB");
            finWait2Timer = new cMessage("FIN-WAIT-2");
            synRexmitTimer = new cMessage("SYN-REXMIT");

            the2MSLTimer->setContextPointer(this);
            connEstabTimer->setContextPointer(this);
            finWait2Timer->setContextPointer(this);
            synRexmitTimer->setContextPointer(this);
        }

        bool DSAK_TcpConnection::processTCPSegment(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address segSrcAddr, L3Address segDestAddr)
        {
            Enter_Method_Silent();

            take(packet);
            printConnBrief();
            if (!localAddr.isUnspecified()) {
                ASSERT(localAddr == segDestAddr);
                ASSERT(localPort == tcpseg->getDestPort());
            }

            if (!remoteAddr.isUnspecified()) {
                ASSERT(remoteAddr == segSrcAddr);
                ASSERT(remotePort == tcpseg->getSrcPort());
            }

            if (tryFastRoute(tcpseg))
                return true;

            // first do actions
            TcpEventCode event = DSAK_TcpConnection::process_RCV_SEGMENT(packet, tcpseg, segSrcAddr, segDestAddr);

            // then state transitions
            return performStateTransition(event);
        }

        bool DSAK_TcpConnection::processAppCommand(cMessage *msg)
        {
            Enter_Method_Silent();

            take(msg);
            printConnBrief();

            // first do actions
            TcpCommand *tcpCommand = check_and_cast_nullable<TcpCommand *>(msg->removeControlInfo());
            TcpEventCode event = preanalyseAppCommandEvent(msg->getKind()); // C 表示 command; E 表示 event
            EV_INFO << "App command: " << eventName(event) << "\n";

            switch (event) {
                case TCP_E_OPEN_ACTIVE:
                    process_OPEN_ACTIVE(event, tcpCommand, msg);
                    break;

                case TCP_E_OPEN_PASSIVE:
                    process_OPEN_PASSIVE(event, tcpCommand, msg);
                    break;

                case TCP_E_ACCEPT:
                    process_ACCEPT(event, tcpCommand, msg);
                    break;

                case TCP_E_SEND:
                    process_SEND(event, tcpCommand, msg);
                    break;

                case TCP_E_CLOSE:
                    process_CLOSE(event, tcpCommand, msg);
                    break;

                case TCP_E_ABORT:
                    process_ABORT(event, tcpCommand, msg);
                    break;

                case TCP_E_DESTROY:
                    process_DESTROY(event, tcpCommand, msg);
                    break;

                case TCP_E_STATUS:
                    process_STATUS(event, tcpCommand, msg);
                    break;

                case TCP_E_QUEUE_BYTES_LIMIT:
                    process_QUEUE_BYTES_LIMIT(event, tcpCommand, msg);
                    break;

                case TCP_E_READ:
                    process_READ_REQUEST(event, tcpCommand, msg);
                    break;

                case TCP_E_SETOPTION:
                    process_OPTIONS(event, tcpCommand, msg);
                    break;

                default:
                    throw cRuntimeError(tcpMain, "wrong event code");
            }

            // then state transitions
            return performStateTransition(event);
        }
    }
}

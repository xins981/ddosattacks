#include <string.h>

#include "inet/applications/common/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

#include "DSAK_TcpConnection.h"

namespace dsak
{
    namespace tcp
    {

        void DSAK_TcpConnection::process_OPEN_ACTIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
        {
            TcpOpenCommand *openCmd = check_and_cast<TcpOpenCommand *>(tcpCommand);
            L3Address localAddr, remoteAddr;
            int localPort, remotePort;

            switch (fsm.getState()) {
                case TCP_S_INIT:
                    initConnection(openCmd);

                    // store local/remote socket
                    state->active = true;
                    localAddr = openCmd->getLocalAddr();
                    remoteAddr = openCmd->getRemoteAddr();
                    localPort = openCmd->getLocalPort();
                    remotePort = openCmd->getRemotePort();

                    if (remoteAddr.isUnspecified() || remotePort == -1)
                        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: remote address and port must be specified");

                    if (localPort == -1) {
                        localPort = tcpMain->getEphemeralPort();
                        EV_DETAIL << "Assigned ephemeral port " << localPort << "\n";
                    }

                    EV_DETAIL << "OPEN: " << localAddr << ":" << localPort << " --> " << remoteAddr << ":" << remotePort << "\n";

                    tcpMain->addSockPair(this, localAddr, remoteAddr, localPort, remotePort);

                    // send initial SYN
                    selectInitialSeqNum();
                    sendSyn();
                    startSynRexmitTimer();
                    scheduleTimeout(connEstabTimer, TCP_TIMEOUT_CONN_ESTAB);
                    break;

                default:
                    throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: connection already exists");
            }

            delete openCmd;
            delete msg;
        }

    } // namespace dsak
} // namespace inet


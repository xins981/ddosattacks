#ifndef __DSAK_TCPCONNECTION_H
#define __DSAK_TCPCONNECTION_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "DSAK_Tcp.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {
        class DSAK_TcpConnection : public TcpConnection
        {
            public:
                void initConnection(DSAK_Tcp *mod, int socketId);
                bool processAppCommand(cMessage *msg) override;
                bool processTCPSegment(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr) override;
                void sendToIP(Packet *packet, const Ptr<TcpHeader>& tcpseg) override;
            protected:
                DSAK_Tcp *tcpMain = nullptr;
            protected:
                TcpEventCode processSegmentInSynSent(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address src, L3Address dest) override;
                TcpEventCode process_RCV_SEGMENT(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address src, L3Address dest) override;
                void configureStateVariables() override;
                void process_OPEN_ACTIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg) override;
                void initConnection(TcpOpenCommand *openCmd) override;
        };
    }
}

#endif // ifndef __DSAK_TCPCONNECTION_H


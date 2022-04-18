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
                using TcpConnection::initConnection;
            protected:
                virtual TcpEventCode processSegmentInSynSent(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address src, L3Address dest) override;
                virtual void initConnection(TcpOpenCommand *openCmd) override;
        };
    }
}

#endif // ifndef __DSAK_TCPCONNECTION_H


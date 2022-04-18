#ifndef __DSAK_TCP_H
#define __DSAK_TCP_H

#include <map>
#include <set>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/base/TransportProtocolBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {
        class DSAK_Tcp : public Tcp
        {
            protected:
                virtual TcpConnection *createConnection(int socketId) override;
        };
    }
}
#endif // ifndef __DSAK_TCP_H


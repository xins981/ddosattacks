#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif // ifdef WITH_IPv6

#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

#include "DSAK_TcpConnection.h"
#include "DSAK_Tcp.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {
        Define_Module(DSAK_Tcp);

        TcpConnection *DSAK_Tcp::createConnection(int socketId)
        {
            auto moduleType = cModuleType::get("ddosattacks.hackedmodules.transportlayer.tcp.DSAK_TcpConnection");
            char submoduleName[24];
            sprintf(submoduleName, "conn-%d", socketId);
            auto module = check_and_cast<DSAK_TcpConnection *>(moduleType->createScheduleInit(submoduleName, this));
            module->initConnection(this, socketId);
            return module;
        }

    }
}


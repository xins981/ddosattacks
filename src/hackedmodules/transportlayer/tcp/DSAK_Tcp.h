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
        // Forward declarations:
        class DSAK_TcpConnection;

        class DSAK_Tcp : public TransportProtocolBase
        {
          public:
            static simsignal_t tcpConnectionAddedSignal;
            static simsignal_t tcpConnectionRemovedSignal;

            enum PortRange {
                EPHEMERAL_PORTRANGE_START = 1024,
                EPHEMERAL_PORTRANGE_END   = 5000
            };

            struct SockPair
            {
                L3Address localAddr;
                L3Address remoteAddr;
                int localPort;    // -1: unspec
                int remotePort;    // -1: unspec

                inline bool operator<(const SockPair& b) const
                {
                    if (remoteAddr != b.remoteAddr)
                        return remoteAddr < b.remoteAddr;
                    else if (localAddr != b.localAddr)
                        return localAddr < b.localAddr;
                    else if (remotePort != b.remotePort)
                        return remotePort < b.remotePort;
                    else
                        return localPort < b.localPort;
                }
            };

          protected:
            typedef std::map<int /*socketId*/, DSAK_TcpConnection *> TcpAppConnMap;
            typedef std::map<SockPair, DSAK_TcpConnection *> TcpConnMap;
            TcpCrcInsertion crcInsertion;

            TcpAppConnMap tcpAppConnMap;
            TcpConnMap tcpConnMap;

            ushort lastEphemeralPort = static_cast<ushort>(-1);
            std::multiset<ushort> usedEphemeralPorts;

          protected:
            /** Factory method; may be overriden for customizing Tcp */
            virtual DSAK_TcpConnection *createConnection(int socketId);

            // utility methods
            virtual DSAK_TcpConnection *findConnForSegment(const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr);
            virtual DSAK_TcpConnection *findConnForApp(int socketId);
            virtual void segmentArrivalWhileClosed(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address src, L3Address dest);
            virtual void refreshDisplay() const override;

          public:
            bool useDataNotification = false;
            CrcMode crcMode = CRC_MODE_UNDEFINED;
            int msl;

          public:
            DSAK_Tcp() {}
            virtual ~DSAK_Tcp();

          protected:
            virtual void initialize(int stage) override;
            virtual int numInitStages() const override { return NUM_INIT_STAGES; }
            virtual void finish() override;

            virtual void handleSelfMessage(cMessage *message) override;
            virtual void handleUpperCommand(cMessage *message) override;
            virtual void handleUpperPacket(Packet *packet) override;
            virtual void handleLowerPacket(Packet *packet) override;

          public:
            /**
             * To be called from TcpConnection when a new connection gets created,
             * during processing of OPEN_ACTIVE or OPEN_PASSIVE.
             */
            virtual void addSockPair(DSAK_TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

            virtual void removeConnection(DSAK_TcpConnection *conn);
            virtual void sendFromConn(cMessage *msg, const char *gatename, int gateindex = -1);

            /**
             * To be called from TcpConnection when socket pair (key for TcpConnMap) changes
             * (e.g. becomes fully qualified).
             */
            virtual void updateSockPair(DSAK_TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

            /**
             * Update conn's socket pair, and register newConn (which'll keep LISTENing).
             * Also, conn will get a new socketId (and newConn will live on with its old socketId).
             */
            virtual void addForkedConnection(DSAK_TcpConnection *conn, DSAK_TcpConnection *newConn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

            /**
             * To be called from TcpConnection: reserves an ephemeral port for the connection.
             */
            virtual ushort getEphemeralPort();

            /**
             * To be called from TcpConnection: create a new send queue.
             */
            virtual TcpSendQueue *createSendQueue();

            /**
             * To be called from TcpConnection: create a new receive queue.
             */
            virtual TcpReceiveQueue *createReceiveQueue();

            // ILifeCycle:
            virtual void handleStartOperation(LifecycleOperation *operation) override;
            virtual void handleStopOperation(LifecycleOperation *operation) override;
            virtual void handleCrashOperation(LifecycleOperation *operation) override;

            // called at shutdown/crash
            virtual void reset();

            bool checkCrc(Packet *pk);
            int getMsl() { return msl; }
        };
    }
}
#endif // ifndef __DSAK_TCP_H


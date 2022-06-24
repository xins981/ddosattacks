#ifndef __DSAK_TCPSYNATTACKAPP_H
#define __DSAK_TCPSYNATTACKAPP_H

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Message.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace dsak
{
    class DSAK_TcpSynAttackApp : public inet::cSimpleModule
    {
        protected:
            inet::TcpSocket socket;
            inet::cMessage *timeoutMsg = nullptr;
            inet::simtime_t startTime;

            void initialize() override;
            void handleMessage(inet::cMessage *msg) override;

    };
}

#endif // ifndef __DSAK_TCPSYNATTACKAPP_H


#include "../../application/tcpapp/DSAK_TcpSynAttackApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"

namespace dsak
{
    Define_Module(DSAK_TcpSynAttackApp);

    void DSAK_TcpSynAttackApp::initialize()
    {

        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);

        socket.setOutputGate(gate("socketOut"));

        startTime = par("startTime");
        timeoutMsg = new inet::cMessage("timer");
        scheduleAt(startTime, timeoutMsg);
    }

    void DSAK_TcpSynAttackApp::handleMessage(inet::cMessage *msg)
    {
        socket.renewSocket();

        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);

        int timeToLive = par("timeToLive");
        if (timeToLive != -1)
            socket.setTimeToLive(timeToLive);

        int dscp = par("dscp");
        if (dscp != -1)
            socket.setDscp(dscp);

        int tos = par("tos");
        if (tos != -1)
            socket.setTos(tos);

        // connect
        const char *connectAddress = par("connectAddress");
        int connectPort = par("connectPort");

        inet::L3Address destination;
        inet::L3AddressResolver().tryResolve(connectAddress, destination);
        socket.connect(destination, connectPort);
    }
}


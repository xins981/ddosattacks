package ddosattacks.hackedmodules.application.tcpapp;

import inet.applications.contract.IApp;

simple DSAK_TcpSynAttackApp like IApp
{
    parameters:
        
        @class(dsak::DSAK_TcpSynAttackApp);
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(-1); // port number to listen on
        string connectAddress = default("");  // server address (may be symbolic)
        double startTime @unit(s) = default(1s);
        int connectPort = default(1000); // port number to connect to
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP (IPv4/IPv6) field of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}


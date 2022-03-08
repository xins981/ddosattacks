package ddosattacks.hackedmodules.networklayer.ipv4;

import inet.networklayer.ipv4.Ipv4;

simple DSAK_Ipv4 extends Ipv4
{
    parameters: 
        //# Labels for the potential attacks that affect this module
        @dropping;
        
        //# Normal parameters
        @display("i=block/routing,red;i2=status/excl3");
	   	@class(DSAK_Ipv4);
	    
	    //# Dropping attack statistics
	    @statistic[dropedPkt](title="Number of drops";source=packetDropped; record=count; interpolationmode=none);
	   	
	   	//@signal[test](type=long);
	   	//@statistic[test](title="test"; record=count, vector; interpolationmode=none);
	   
	    //# Packet received attack statistics
	    @signal[packetReceived](type=long);
	    @statistic[rcvdPkt](title="Number of packet received";source=packetReceived; record=count, vector; interpolationmode=none);
}

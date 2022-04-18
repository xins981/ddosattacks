package ddosattacks.simulations.TcpSynAttackScenario;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EtherSwitch;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.applications.pingapp.PingApp;
import inet.linklayer.ethernet.EtherBus;
import inet.node.ethernet.EtherLink;

network TcpSynAttack
{
    parameters:
        int numSubnet1Attackers;
        int numSubnet2Attackers;
        int numSubnet3Attackers;
    types:
        channel Eth1M extends EtherLink
        {
            length = 20m;
        	datarate = 1Mbps;
        }
        channel Eth500K extends EtherLink
        {
            length = 20m;
        	datarate = 0.5Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator;
        交换机[3]: EtherSwitch;
        路由器[3]: Router; // predefined router
        被控主机[numSubnet1Attackers + numSubnet2Attackers + numSubnet3Attackers]: StandardHost
		{
            @display("i=device/laptop,red;");
        };
        服务器: StandardHost
        {
            @display("i=device/server");
        };
        visualizer: IntegratedVisualizer;
    connections: // network level connections
        for i=0..2
        {
            路由器[i].pppg++ <--> Eth500K <--> 路由器[(i + 1) % 3].pppg++;
        };
        路由器[0].ethg++ <--> Eth1M <--> 交换机[0].ethg++;
        路由器[1].ethg++ <--> Eth1M <--> 交换机[1].ethg++;
        路由器[2].ethg++ <--> Eth1M <--> 交换机[2].ethg++;
        服务器.ethg++ <--> Eth1M <--> 交换机[1].ethg++;
        for i=0..numSubnet1Attackers - 1
        {
            被控主机[i].ethg++ <--> Eth1M <--> 交换机[0].ethg++; // ethernet
        }
        for i=0..numSubnet2Attackers - 1
        {
            被控主机[i + numSubnet1Attackers].ethg++ <--> Eth1M <--> 交换机[1].ethg++; // ethernet
        }
        for i=0..numSubnet3Attackers - 1
        {
            被控主机[i + numSubnet1Attackers + numSubnet2Attackers].ethg++ <--> Eth1M <--> 交换机[2].ethg++; // ethernet
        }
}
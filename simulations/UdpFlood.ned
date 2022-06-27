package ddosattacks.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.node.ethernet.EtherSwitch;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.node.ethernet.Eth10M;

network UdpFlood
{
    parameters:
        int clients_lan1; 
        int clients_lan2;
        int clients_lan3;  
        int attackers_lan1;
        int attackers_lan2;
        int attackers_lan3;
    submodules:
        configurator: Ipv4NetworkConfigurator;
        交换机[3]: EtherSwitch;
        路由器[3]: Router;
        普通用户[clients_lan1 + clients_lan2 + clients_lan3]: StandardHost;
        受控主机[attackers_lan1 + attackers_lan2 + attackers_lan3]: StandardHost
		{
            @display("i=device/laptop,red;");
        };
        服务器: StandardHost
        {
            @display("i=device/server");
            @statistic[NumReceivedIpPackets](source=count(packetReceivedFromLower); record=vector);
        };
        visualizer: IntegratedVisualizer;
    connections:
        for i=0..2
        {
            路由器[i].ethg++ <--> Eth10M <--> 路由器[(i + 1) % 3].ethg++;
        }
        
        for i=0..2
        {
            路由器[i].ethg++ <--> Eth10M <--> 交换机[i].ethg++;
        }
        
        for i=0..clients_lan1 - 1
        {
            普通用户[i].ethg++ <--> Eth10M <--> 交换机[0].ethg++;
        }
        for i=0..attackers_lan1 - 1
        {
            受控主机[i].ethg++ <--> Eth10M <--> 交换机[0].ethg++;
        }
        
        for i=0..clients_lan2 - 1
        {
            普通用户[i + clients_lan1].ethg++ <--> Eth10M <--> 交换机[1].ethg++;
        }
        for i=0..attackers_lan2 - 1
        {
            受控主机[i + attackers_lan1].ethg++ <--> Eth10M <--> 交换机[1].ethg++;
        }
        
        for i=0..clients_lan3 - 1
        {
            普通用户[i + clients_lan1 + clients_lan2].ethg++ <--> Eth10M <--> 交换机[2].ethg++;
        }
        for i=0..attackers_lan3 - 1
        {
            受控主机[i + attackers_lan1 + attackers_lan2].ethg++ <--> Eth10M <--> 交换机[2].ethg++;
        }
        
        服务器.ethg++ <--> Eth10M <--> 交换机[0].ethg++;
}

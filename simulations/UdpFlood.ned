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
        int intClientsInFirstLan; 
        int intClientsInSecondLan;
        int intClientsInThirdLan;
        int intAttackersInFirstLan;
        int intAttackersInSecondLan;
        int intAttackersInFThirdLan;
    submodules:
        configurator: Ipv4NetworkConfigurator;
        交换机[3]: EtherSwitch;
        路由器[3]: Router;
        普通用户[intClientsInFirstLan + intClientsInSecondLan + intClientsInThirdLan]: StandardHost;
        受控主机[intAttackersInFirstLan + intAttackersInSecondLan + intAttackersInFThirdLan]: StandardHost
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
        
        for i=0..intClientsInFirstLan - 1
        {
            普通用户[i].ethg++ <--> Eth10M <--> 交换机[0].ethg++;
        }
        for i=0..intAttackersInFirstLan - 1
        {
            受控主机[i].ethg++ <--> Eth10M <--> 交换机[0].ethg++;
        }
        
        for i=0..intClientsInSecondLan - 1
        {
            普通用户[i + intClientsInFirstLan].ethg++ <--> Eth10M <--> 交换机[1].ethg++;
        }
        for i=0..intAttackersInSecondLan - 1
        {
            受控主机[i + intAttackersInFirstLan].ethg++ <--> Eth10M <--> 交换机[1].ethg++;
        }
        
        for i=0..intClientsInThirdLan - 1
        {
            普通用户[i + intClientsInFirstLan + intClientsInSecondLan].ethg++ <--> Eth10M <--> 交换机[2].ethg++;
        }
        for i=0..intAttackersInFThirdLan - 1
        {
            受控主机[i + intAttackersInFirstLan + intAttackersInSecondLan].ethg++ <--> Eth10M <--> 交换机[2].ethg++;
        }
        
        服务器.ethg++ <--> Eth10M <--> 交换机[0].ethg++;
}

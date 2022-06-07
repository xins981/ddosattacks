//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package ddosattacks.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.node.ethernet.EtherSwitch;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.applications.pingapp.PingApp;
import inet.linklayer.ethernet.EtherBus;
import inet.node.ethernet.EtherLink;

network Ping攻击
{
    parameters:
        int numSubnet1Clients; // number of clients in the network
        int numSubnet2Clients;
        int numSubnet3Clients;
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
        交换机: EtherSwitch;
        总线[2]: EtherBus;
        路由器[3]: Router; // predefined router
        正常客户[numSubnet1Clients + numSubnet2Clients + numSubnet3Clients]: StandardHost;
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
        路由器[0].ethg++ <--> Eth1M <--> 交换机.ethg++;
        路由器[1].ethg++ <--> Eth1M <--> 总线[0].ethg++;
        路由器[2].ethg++ <--> Eth1M <--> 总线[1].ethg++;
        服务器.ethg++ <--> Eth1M <--> 总线[1].ethg++;
        for i=0..numSubnet1Clients - 1
        {
            正常客户[i].ethg++ <--> Eth1M <--> 交换机.ethg++; // ethernet
        }
        for i=0..numSubnet2Clients - 1 
        {
            正常客户[i + numSubnet1Clients].ethg++ <--> Eth1M <--> 总线[0].ethg++; // ethernet
        }
        for i=0..numSubnet3Clients - 1
        {
            正常客户[i + numSubnet1Clients + numSubnet2Clients].ethg++ <--> Eth1M <--> 总线[1].ethg++; // ethernet
        }
        for i=0..numSubnet1Attackers - 1
        {
            被控主机[i].ethg++ <--> Eth1M <--> 交换机.ethg++; // ethernet
        }
        for i=0..numSubnet2Attackers - 1
        {
            被控主机[i + numSubnet1Attackers].ethg++ <--> Eth1M <--> 总线[0].ethg++; // ethernet
        }
        for i=0..numSubnet3Attackers - 1
        {
            被控主机[i + numSubnet1Attackers + numSubnet2Attackers].ethg++ <--> Eth1M <--> 总线[1].ethg++; // ethernet
        }
}

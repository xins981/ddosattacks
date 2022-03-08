package ddosattacks.simulations.SimpleAttackScenarios.SimpleDroppingAttackScenario;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth1G;
import inet.node.inet.Router;
import inet.node.ethernet.EtherSwitch;
import inet.visualizer.integrated.IntegratedVisualizer;

import ddosattacks.nodes.DSAK_StandardHost;

network SimpleDropping
{
    parameters:
        int numHosts; // number of clients in the network
    submodules:
        configurator: Ipv4NetworkConfigurator;
        switch: EtherSwitch;
        router: Router; // predefined router
        client[numHosts]: StandardHost;
        server: DSAK_StandardHost
        {
            @display("i=device/server");
        };
        visualizer: IntegratedVisualizer;
    connections: // network level connections
        for i=0..numHosts-1 
        {
            client[i].ethg++ <--> Eth1G <--> switch.ethg++; // ethernet
        }
        server.ethg++ <--> Eth1G <--> router.ethg++;
        switch.ethg++ <--> Eth1G <--> router.ethg++;
}

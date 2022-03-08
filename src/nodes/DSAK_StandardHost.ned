package ddosattacks.nodes;

import inet.node.inet.StandardHost;

import ddosattacks.attacks.controllers.droppingAttack.DSAK_DroppingAttack;


module DSAK_StandardHost extends StandardHost
{
    @display("i=,red");
    submodules:
        droppingAttack: DSAK_DroppingAttack 
        {
            @display("p=1126.125,77.174995");
        }
}


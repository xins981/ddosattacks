package ddosattacks.attacks.controllers.droppingAttack;

import ddosattacks.attacks.controllers.DSAK_Attack;

simple DSAK_DroppingAttack extends DSAK_Attack
{
    parameters:
        @class(DSAK_DroppingAttack);
        @display("i=misc/cloud3,red,100;is=l");

        attackType = "dropping"; 
        double droppingAttackProbability = default(0);
}

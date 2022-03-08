#include "DSAK_DroppingAttack.h"
#include "DSAK_DroppingMessage_m.h"

Define_Module (DSAK_DroppingAttack);

cMessage *DSAK_DroppingAttack::generateAttackMessage(const char *name)
{
    LOG << "DSAK_DroppingAttack: generateAttackMessage\n";

    // Specific message for the specifics hacked modules.
    DSAK_DroppingMessage *msg = new DSAK_DroppingMessage(name);
    msg->setDroppingAttackProbability(par("droppingAttackProbability").doubleValue());

    return msg;
}

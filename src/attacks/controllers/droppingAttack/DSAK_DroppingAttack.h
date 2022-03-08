#ifndef NA_DROPPINGATTACK_H_
#define NA_DROPPINGATTACK_H_

#include "../DSAK_Attack.h"
#include "DSAK_DroppingMessage_m.h"

class DSAK_DroppingAttack: public DSAK_Attack
{
    protected:
        virtual cMessage *generateAttackMessage(const char* name);
};

#endif

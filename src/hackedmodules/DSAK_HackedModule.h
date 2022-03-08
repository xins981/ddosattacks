#ifndef NA_HACKEDMODULE_H_
#define NA_HACKEDMODULE_H_

#include "../common/log/DSAK_Log.h"

class DSAK_HackedModule
{
    private:
        DSAK_Log log;
    public:
        virtual void handleMessageFromAttackController(cMessage *msg);
};

#endif

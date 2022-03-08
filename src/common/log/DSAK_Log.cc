#include "DSAK_Log.h"

DSAK_Log::DSAK_Log() {
    active = true;
}

void DSAK_Log::write(const char* logline, ...) {
    if (active) {
        va_list argList;
        char cbuffer[1024];
        va_start(argList, logline);
        vsnprintf(cbuffer, 1024, logline, argList);
        va_end(argList);

        EV << DSAKLOGHEADER << cbuffer << endl;
    }
}

void DSAK_Log::unsetLog() {
    active = false;
}


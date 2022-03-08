#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>
#include "DSAK_Utils.h"

std::string DSAK_Utils::currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}


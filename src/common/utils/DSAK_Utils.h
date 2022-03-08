#include <string>

#ifndef DSAK_UTILS_H_
#define DSAK_UTILS_H_

class DSAK_Utils
{
    public:

        /**
         * Get current date/time, format is YYYY-MM-DD.HH:mm:ss
         *
         * @return string time in the specific format
         */
        static std::string currentDateTime();
};

#endif

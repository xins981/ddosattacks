#ifndef DSAKLOG_H_
#define DSAKLOG_H_

#include <fstream>
#include <omnetpp.h>
#include <stdarg.h>

#include "../utils/DSAK_Utils.h"

using namespace std;
using namespace omnetpp;

/** LOG header*/
#define DSAKLOGHEADER "[" << DSAK_Utils::currentDateTime() << "][" << simTime() << "][DSAKLOG]: "
/** LOG macro*/
#define LOG log << DSAKLOGHEADER

class DSAK_Log
{
    public:
        /**
         * Constructor
         */
        DSAK_Log();

        /**
         * Destructor
         */
        ~DSAK_Log() {
        }

        /**
         * Write a log line in the simulation console
         *
         * @param logline
         */
        void write(const char* logline, ...);

        /**
         * Deactivate the logs
         */
        void unsetLog();

        /**
         * Override the operator << for strings
         *
         * @param t the string to write on log
         * @return
         */
        DSAK_Log& operator<<(const std::string& t) {
            if (active)
                EV << t;
            return *this;
        } // For strings

        /**
         * Override the operator << for generic objects
         *
         * @param t a generic object to write on log
         * @return
         */
        template<typename T> DSAK_Log& operator<<(const T& t) {
            if (active)
                EV << t;
            return *this;
        } // For generic objects

        /**
         * Override the operator << for objects like endl
         *
         * @param t
         * @return
         */
        DSAK_Log& operator<<(std::ostream& (t)(std::ostream&)) {
            if (active)
                EV << t;
            return *this;
        } // For objects like endl

    private:
        /**
         * Flag to active the logs or not
         */
        bool active; // Indicates if logging is active or not

};

#endif

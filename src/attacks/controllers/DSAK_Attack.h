#ifndef DSAK_ATTACK_H_
#define DSAK_ATTACK_H_

#include <omnetpp.h>

#include "../../common/log/DSAK_Log.h"

using namespace omnetpp;

/* --------------- CONSTANTS --------------------------------*/
/**
 * Constant for tag Activate in the attack controller message name
 */
#define DSAK_ATTACK_ACTIVATE_TAG "Activate"

/**
 * Constant for tag Deactivate in the attack controller message name
 */
#define DSAK_ATTACK_DEACTIVATE_TAG "Deactivate"

/**
 * Constant for the EndAttack message name
 */
#define DSAK_ATTACK_END_MESSAGE "EndAttack"

/**
 * Constant for the StartAttack message name
 */
#define DSAK_ATTACK_START_MESSAGE "StartAttack"

/* ---------------- NED MODULE ATTRIBUTES NAMES ------------------ */
/**
 * Constant for the type of the attack parameter
 */
#define DSAK_ATTACK_TYPE "attackType"

/**
 * Constant for the name of the active parameter
 */
#define DSAK_ATTACK_ACTIVE "active"

/**
 * Constant for the name of the end time parameter
 */
#define DSAK_ATTACK_END_TIME "endTime"

/**
 * Constant for the name of the start time parameter
 */
#define DSAK_ATTACK_START_TIME "startTime"


class DSAK_Attack: public cSimpleModule
{

    private:
        /**
         * String identifying the kind of attack to be striked
         */
        char* attackType;

        /**
         * List of modules for activation of the attack
         */
        vector<cModule *> modList;

    protected:

        /**
         * Log reference
         */
        DSAK_Log log;

        /**
         * Method from cSimpleModule class, to initialize the simple module.
         */
        virtual void initialize();

        /**
         * Get the modules (hacked modules) in which the attack should be activated (saved in modList)
         *
         */
        void getAttackModules();

        /**
         * Schedule the activation/deactivation time of the attack
         */
        void scheduleAttack();

        /**
         * For send the corresponding message to the specifics hacked modules implied in the attack
         *
         * @param msg cMessage, the message of the specific attack and with its specific attributes.
         */
        void sendMessageToHackedModules(cMessage *msg);

        /**
         * Activate the attack in the hacked modules.
         */
        virtual void activateModules();

        /**
         * Deactivate the attack in the hacked modules
         */
        virtual void deactivateModules();

        /**
         * Method from cSimpleModule that is listening all messages scheduling by the scheduleAt method.
         *
         * @param msg cMessage, the received message.
         */
        virtual void handleMessage(cMessage *msg);

        /**
         * Generate the specific message for an attack. This must be overridden by all attack controllers subclasses.
         * The name of the message, will be automatic generated by DSAK_ATTACK in the following way:
         * @code
         *
         * <attackType> + <Activate/Deactivate>
         *
         * @endcode
         *
         * For example, in the dropping attack case, the name of the activation message is "droppingActivate".
         *
         * @param name char, name of the message.
         * @return cMessage, the generated message.
         */
        virtual cMessage *generateAttackMessage(const char* name);
};

#endif /* DSAK_ATTACK_H_ */

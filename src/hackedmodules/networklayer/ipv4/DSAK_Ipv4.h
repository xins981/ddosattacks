#ifndef DSAK_IPV4_H_
#define DSAK_IPV4_H_

#include "Ipv4.h"
#include <omnetpp.h>

#include "../../../common/log/DSAK_Log.h"
#include "../../DSAK_HackedModule.h"
#include "DSAK_DroppingMessage_m.h"

using namespace inet;
/**
 * Constant to distinguish TCP packet data payload instead of ACK or SYN packets
 */
#define TCP_DATA "tcp"

/**
 * Constant to distinguish UDP packets data.
 */
#define UDP_DATA "UDP"

/**
 * Constant to distinguish PING application data packets.
 */
#define PING_DATA "ping"

/**
 * @brief Dropping attack hacked module
 *
 * @details This hacked module is in charge of implement the dropping behavior on
 * IP layer. When this module receive a dropping control message from the controller
 * this activate or deactivate the dropping behavior. The packets are discarded randomly
 * following a normal distribution with a @verbatim droppingAttackProbability @endverbatim
 * probability.
 *
 * Implemented attacks:
 * - Dropping
 * - Delay
 *
 * @see NA_HackedModule, NA_DroppingAttack
 *
 * @author Gabriel Maciá Fernández, gmacia@ugr.es
 * @date 01/22/2013
 *
 */
class DSAK_Ipv4 : public Ipv4, public DSAK_HackedModule
{

private:

    /**
     * Log reference
     */
    DSAK_Log log;

    /*----------------- DROPPING ATTACK  -------------------------*/
    /**
     * Flag to activate dropping attack
     */
    bool droppingAttackIsActive;

    /**
     * Probability for dropping packets when dropping attack is active
     */
    double droppingAttackProbability;
    long numRcvdPkt;
    // long numtest;
    bool isValidPkt;
public:

    /**
     * Overridden function
     */
    void handleMessageFromAttackController(cMessage *msg);

protected:

    /**
     * Method from cSimpleModule class, to initialize the simple module.
     * Overridden function.
     */
    virtual void initialize();

    /**
     * Overridden function to implement the dropping behavior.
     * First check if the dropping behavior is active. Then check if the received packet is
     * a valid packet to drop (PING, UDP and/or TCP). Finally discard it or not randomly.
     */
    // virtual void handlePacketFromNetwork(IPv4Datagram *datagram, InterfaceEntry *fromIE);
    virtual void handleIncomingDatagram(Packet *packet);

    // simsignal_t testSignal;

};
#endif /* NA_IPV4_H_ */

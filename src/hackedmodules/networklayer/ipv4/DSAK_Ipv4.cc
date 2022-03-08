#include <iostream>
#include <string.h>

#include "DSAK_Ipv4.h"

using std::cout;

Define_Module(DSAK_Ipv4);

void DSAK_Ipv4::initialize()
{
    // Dropping attack initialization
    numRcvdPkt = 0;
    droppingAttackProbability = 0;
    droppingAttackIsActive = isValidPkt = false;

    // numtest = 0;
    //testSignal = registerSignal("test");
    //subscribe(testSignal, this);
    Ipv4::initialize(INITSTAGE_LOCAL);
}

void DSAK_Ipv4::handleMessageFromAttackController(cMessage *msg) {

    // It is necessary to call Enter_Method for doing context switching (4.10 of User Manual)
    Enter_Method("DSAK_Ipv4: handle message from attack controller");

    LOG << "DSAK_Ipv4: Received message: " << msg->getFullName() << "\n";

    /*-------------------------- DROPPING ATTACK -------------------------*/
    if (not strcmp(msg->getFullName(), "droppingActivate"))
    {
        DSAK_DroppingMessage *dmsg;
        dmsg = check_and_cast<DSAK_DroppingMessage *>(msg);
        LOG << "--> Activating module DSAK_Ipv4 for Dropping Attack...\n";
        LOG << "    Dropping Attack Probability received: "
                << dmsg->getDroppingAttackProbability() << "\n";
        //Now dropping attack is activated in this module
        droppingAttackIsActive = true;
        droppingAttackProbability = dmsg->getDroppingAttackProbability();
        delete (msg);
    }
    else if (not strcmp(msg->getFullName(), "droppingDeactivate"))
    {
        DSAK_DroppingMessage *dmsg;
        dmsg = check_and_cast<DSAK_DroppingMessage *>(msg);
        LOG << "Deactivating module DSAK_Ipv4 for Dropping Attack...\n";
        //Now dropping attack is deactivated
        droppingAttackIsActive = false;
        delete (msg);
    }
    else
    {
        LOG << "ERROR: Message unknown in DSAK_Ipv4::handleMessageFromAttackController. Msg: " << msg->getFullName() << "\n";
    }
}

void DSAK_Ipv4::handleIncomingDatagram(Packet* packet)
{
    ASSERT(packet);

    //cout << simTime() << ": Incoming packet: " << packet->getFullPath() << endl;
    //cout << simTime() << ": Source address: " << packet->getSrcAddress().str() << endl;
    //cout << simTime() << ": Destination address: " << packet->getDestAddress().str() << endl;

    // Count the number of total data packet received, for statistics.
    isValidPkt = (!strncmp(packet->getName(), PING_DATA, 4) || !strncmp(packet->getName(), UDP_DATA, 3) || !strncmp(packet->getName(), TCP_DATA, 3));
    if (isValidPkt)
    {
        emit(packetReceivedSignal, ++numRcvdPkt); // Sending of the signal indicating that we have received a new data packet.
    }

    //Packet is a ping/UDP/TCP (data packet)
    if (droppingAttackIsActive)
    {
        LOG << "Received packet after activating dropping attack ... " << "\n";
        if (isValidPkt)
        {
            LOG << "Is a valid packet for dropping ..." << "\n";
            if (uniform(0, 1) < droppingAttackProbability)
            {
                // emit(testSignal, ++numtest);
                LOG << "Discarding packet: " << packet->getName() << ": " << ++numDropped << " dropping times." << endl;
                cout << simTime() << ": Discarding packet: " << packet->getName() << endl;
                PacketDropDetails details;
                details.setReason(FORWARDING_DISABLED);
                emit(packetDroppedSignal, packet, &details);
                delete packet; //Deletes the packet thus calling its destructor
                return;
            }
            else
            {
                Ipv4::handleIncomingDatagram(packet);
            }
        }
        else
        { //Packet is not a data packet --> normal behavior
            Ipv4::handleIncomingDatagram(packet);
        }
    }
    else { // --> Normal behavior.
        Ipv4::handleIncomingDatagram(packet);
        // Ipv4::handlePacketFromNetwork(packet, fromIE);
    }
}

#include "../../common/log/DSAK_Log.h"
#include "../../hackedmodules/DSAK_HackedModule.h"
#include "DSAK_Attack.h"

Define_Module (DSAK_Attack);

void DSAK_Attack::initialize() {

    // Get the type of attack to be launched
    attackType = (char*) par(DSAK_ATTACK_TYPE).stringValue();

    // Activate the attack only if defined in the active parameter in module (.ned)
    if (par(DSAK_ATTACK_ACTIVE).boolValue() == true) {

        getAttackModules();
        scheduleAttack();
    }
}

void DSAK_Attack::getAttackModules() {
    cTopology topo; //Used to discover the topology of the node and find modules for activating the attack
    cTopology::Node *node;
    string nodeName;

    // extract all modules with the @attackType property set in the simulation
    topo.extractByProperty(par(DSAK_ATTACK_TYPE).stringValue());

    LOG << "------------------------------------\n";
    LOG << "Found " << topo.getNumNodes() << " possible modules for attack\n";
    LOG << "------------------------------------\n";

    // Now, only the modules contained in the parent module of this DSAK_ATTACK object are activated.
    string prefix = this->getParentModule()->getFullPath(); // First we get the full path of the parent node
    int numModules = 0;
    for (int i = 0; i < topo.getNumNodes(); i++) {
        node = topo.getNode(i);
        nodeName = node->getModule()->getFullPath();
        if (not nodeName.compare(0, prefix.size(), prefix)) {

            LOG << "--->Inserting module in list: " << nodeName << "\n";
            modList.push_back(node->getModule());
            numModules++;
        }
    }
    LOG << "-----------------------------------\n";
    LOG << "Inserted " << numModules << " modules in list\n";
    LOG << "-----------------------------------\n";
}

void DSAK_Attack::scheduleAttack() {
    cMessage *msg = new cMessage(DSAK_ATTACK_START_MESSAGE);
    LOG << "Scheduling the attack \n";
    scheduleAt(par(DSAK_ATTACK_START_TIME).doubleValue(), msg);
    if (par("endTime").doubleValue()) //When the value differs from 0
    {
        cMessage *msgEnd = new cMessage(DSAK_ATTACK_END_MESSAGE);
        scheduleAt(par(DSAK_ATTACK_END_TIME).doubleValue(), msgEnd);
    }
}

void DSAK_Attack::handleMessage(cMessage *msg) {
    LOG << "Message received: " << msg->getFullName() << "\n";
    if (not strcmp(msg->getFullName(), DSAK_ATTACK_START_MESSAGE)) {
        activateModules();
    } else {
        deactivateModules();
    }
    delete (msg);
}

void DSAK_Attack::activateModules() {
    char msgCaption[30];

    // Concatenate the <attackType> + Activate
    opp_strcpy(msgCaption, attackType);
    strcat(msgCaption, DSAK_ATTACK_ACTIVATE_TAG);

    // Generate the specific attack controller message.
    // This method belongs to the specific attack controller.
    cMessage *msg = check_and_cast<cMessage *>(generateAttackMessage(msgCaption));
    EV << "\n\n";
    LOG << "-----------------------------------\n";
    LOG << "ACTIVATING HACKED MODULES\n";
    LOG << "-----------------------------------\n";

    sendMessageToHackedModules(msg);
}

void DSAK_Attack::deactivateModules() {

    char msgCaption[30];

    // Concatenate the <attackType> + Activate
    opp_strcpy(msgCaption, attackType);
    strcat(msgCaption, DSAK_ATTACK_DEACTIVATE_TAG);

    // Generate the specific attack controller message.
    // This method belongs to the specific attack controller.
    cMessage *msg = check_and_cast<cMessage *>(
            generateAttackMessage(msgCaption));

    EV << "\n\n";
    LOG << "-----------------------------------\n";
    LOG << "DEACTIVATING HACKED MODULES\n";
    LOG << "-----------------------------------\n";

    sendMessageToHackedModules(msg);
}

void DSAK_Attack::sendMessageToHackedModules(cMessage *msg) {

    unsigned int i;
    DSAK_HackedModule *modHacked;

    for (i = 0; i < modList.size(); i++) {
        LOG << "Module: " << modList[i]->getFullPath() << "\n";
        if (modList[i]->isSimple())
        { // Activation is only done in simple modules (implemented in C++ classes).

            modHacked = dynamic_cast<DSAK_HackedModule *>(modList[i]);

            LOG << "--> Sending message: " << msg->getFullName() << "\n";
            // Send the message to the specific hacked module
            modHacked->handleMessageFromAttackController(msg);
        } else {
            LOG << "--> Message not sent. Not a simple module.\n";
        }
    }
    LOG << "-----------------------------------\n";
}

cMessage *DSAK_Attack::generateAttackMessage(const char *name) {

    LOG << "ERROR: EN DSAK_ATTACK GENERATE ATTACK MESSAGE\n";
    cMessage *msg = new cMessage(name);
    return msg;
}

#include <string.h>
#include <algorithm>    // min,max

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"

#include "DSAK_TcpConnection.h"

using namespace inet;
using namespace inet::tcp;

namespace dsak
{
    namespace tcp
    {
        void DSAK_TcpConnection::initConnection(TcpOpenCommand *openCmd)
        {
            // create send queue
            sendQueue = tcpMain->createSendQueue();
            sendQueue->setConnection(this);

            // create receive queue
            receiveQueue = tcpMain->createReceiveQueue();
            receiveQueue->setConnection(this);

            // create SACK retransmit queue
            rexmitQueue = new TcpSackRexmitQueue();
            rexmitQueue->setConnection(this);

            // create algorithm
            const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();

            if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
                tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");

            tcpAlgorithm = check_and_cast<TcpAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass, "inet::tcp::TcpConnection"));
            tcpAlgorithm->setConnection(this);

            // create state block
            state = tcpAlgorithm->getStateVariables();
            configureStateVariables();
            tcpAlgorithm->initialize();
        }
    }
}


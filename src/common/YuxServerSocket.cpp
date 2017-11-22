#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "YuxServerSocket.h"
#include "base/Log.h"
#include "pbout/message.pb.h"
#include "statemachine/StateMachine.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace common{

YuxServerSocket* YuxServerSocket::create(const char* host, uint16_t port)
{
    YuxServerSocket* pSock = new YuxServerSocket(host, port);
    pSock->setCbRead(std::tr1::bind(&YuxServerSocket::readCallBack, pSock, _1, _2, _3));
    return pSock;
}

int YuxServerSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    std::auto_ptr<msg::Msg> msg(new msg::Msg());
    if (!msg->ParseFromArray(buf, size))
    {
        cout<<"msg parse error "<<"\n";
        return -1;
    }

    const msg::Header& header = msg->header();;
    uint32_t msgType = header.type();
    uint32_t smId = header.sm_id();
    uint32_t actionId = header.action_id();
    if (!SMFactory::getInstance().isValidSmType(msgType))
    {
        cout<<"Invalid msg type:"<<msgType<<"\n";
        return 0;
    }

    static uint32_t uuid = 0;
    StateMachineBase* pSM = SMFactory::getInstance().getSM(smId);
    if (!pSM)
    {
        pSM = SMFactory::getInstance().createSM(msgType);
        SMFactory::getInstance().addSM(++uuid, pSM); //assign an unique id;
    }

    char evDesc[100];
    snprintf(evDesc, 100, "Message type: %d action Id %d", msgType, actionId);

    Event ev(actionId, evDesc);
    pSM->Drive(ev, sock);
    return 1;
}

}}

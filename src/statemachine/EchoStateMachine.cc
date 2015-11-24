#include "EchoStateMachine.h"
#include "../pbout/echo.pb.h"

SM_REG(MYID, EchoServer);
SM_CB(EchoServer::STATE_INIT, EchoServer, Init);
SM_CB(EchoServer::STATE_WAIT, EchoServer, Response);

using namespace msg;

int EchoServer::Init(const Event &event, SocketBase* sock)
{
    if (state_ != STATE_INIT)
    {
        cout<<"Error state in message, just ignore\n";
        return 0;
    }
    cout<<__func__<<": eventId "<<event.id_<<" eventDesc: "<<event.desc_<<"\n";

    Msg msg;
    Header* hdr = msg.mutable_header();
    hdr->set_version(0);
    hdr->set_type(type_);
    hdr->set_sm_id(id_);

    Body* body = msg.mutable_body();
    body->SetExtension(echo::message, event.desc_);

    uint32_t messageSize = msg.ByteSize();
    uint8_t* buffer      = NULL;

    buffer = new uint8_t[messageSize];
    msg.SerializeWithCachedSizesToArray(buffer);

    sock->send(buffer, messageSize);
    state_ = STATE_WAIT;
    return 0;
}

int EchoServer::Response(const Event &event, SocketBase* sock)
{
    if (state_ != STATE_WAIT)
    {
        cout<<"Error state in message, just ignore\n";
        return 0;
    }

    VBuff v((const uint8_t*)"123", 4);
    VBuff v1(v, 1, 2);
    cout<<__func__<<" "<<event.desc_<<"\n";
    return 1;
}

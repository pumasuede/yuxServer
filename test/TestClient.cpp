#include <iostream>
#include "base/Socket.h"
#include "pbout/message.pb.h"

using namespace std;
using namespace msg;
using namespace yux::base;

int main(int argc, char** argv)
{
    Socket sock;
    sock.connect("127.0.0.1", 8000);

    Msg msg;
    Header* hdr = msg.mutable_header();
    hdr->set_version(1);
    hdr->set_type(1000);
    int  smId = (argc > 1) ? atoi(argv[1]) : 0 ;
    hdr->set_sm_id(smId);
    int  actionId = (argc > 2) ? atoi(argv[2]) : 0 ;
    cout<<"actionId: "<<actionId<<"\n";
    hdr->set_action_id(actionId);

    uint32_t messageSize = msg.ByteSize();
    uint8_t* buffer      = NULL;

    buffer = new uint8_t[messageSize];
    msg.SerializeWithCachedSizesToArray( buffer );
    sock.send(buffer, messageSize);

    uint8_t recvBuf[10000];
    Msg recvMsg;
    int ret = sock.recv(recvBuf, sizeof(recvBuf));
    recvMsg.ParseFromArray(recvBuf, ret);
    cout<<"received msg:\n"<<recvMsg.DebugString()<<"\n";
    while (1)
    {
        sleep(10);
    }

    //log_debug("line %d\n", 1);
}

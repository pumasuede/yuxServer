#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <sstream>

#include "base/VBuff.h"
#include "statemachine/StateMachine.h"

using namespace std;
static uint32_t MYID = 1000;

class EchoServer: public StateMachine<EchoServer>
{
public:
    typedef enum
    {
        STATE_INIT = -1,
        STATE_WAIT = 1
    }  enumState;

    int Init(const Event &event, SocketBase* sock);
    int Response(const Event &event, SocketBase* sock);
    uint32_t MachineType() { return MYID; }
};

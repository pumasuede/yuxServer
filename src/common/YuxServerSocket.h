#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "base/Socket.h"

namespace yux{
namespace common{

class YuxServerSocket : public yux::base::ServerSocket
{
public:
    static int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock, void *pArgs);
    YuxServerSocket(const char* host, uint16_t port, yux::base::SocketBase::CbFun cbRead=&readCallBack) : ServerSocket(host, port, cbRead) {}
};

}}

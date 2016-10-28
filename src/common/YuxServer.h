#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "base/Server.h"

namespace yux{
namespace common{

class YuxServer : public yux::base::Server
{
public:
    static int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock, void *pArgs);
    YuxServer(std::string host, uint16_t port, yux::base::SocketBase::CbFun cbRead=&readCallBack) : Server(host, port, cbRead) {}
};

}}

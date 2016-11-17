#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "base/Socket.h"
#include "parser/HttpParser.h"

namespace yux{
namespace common{

class HttpServerSocket : public yux::base::ServerSocket
{
public:
    static int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock, void *pArgs);
    HttpServerSocket(const char* host, uint16_t port = 80, yux::base::SocketBase::CbFun cbRead=&readCallBack) : ServerSocket(host, port, cbRead) {}

private:
    yux::parser::HttpRequestParser httpParser_;
};

}}

#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "base/Server.h"
#include "parser/HttpParser.h"

namespace yux{
namespace common{

class HttpServer : public yux::base::Server
{
public:
    static int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock, void *pArgs);
    HttpServer(std::string host, uint16_t port = 80, yux::base::SocketBase::CbFun cbRead=&readCallBack) : Server(host, port, cbRead) {}

private:
    yux::parser::HttpRequestParser httpParser_;
};

}}

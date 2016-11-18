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
    int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock);
    static HttpServerSocket* create(const char* host, uint16_t port = 80);

private:
    HttpServerSocket(const char* host, uint16_t port) : ServerSocket(host, port) {}
    yux::parser::HttpRequestParser httpParser_;
};

}}

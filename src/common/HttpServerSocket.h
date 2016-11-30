#include <unistd.h>
#include <stdint.h>

#include "base/Socket.h"
#include "parser/HttpParser.h"

namespace yux{
namespace common{

class HttpServerSocket : public yux::base::ServerSocket
{
public:
    static HttpServerSocket* create(const std::string& host, uint16_t port = 80);
    int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock);
    void setDocRoot(const std::string& docRoot);

private:
    HttpServerSocket(const std::string& host, uint16_t port) : ServerSocket(host.c_str(), port), docRoot_(".") {}
    yux::parser::HttpRequestParser httpParser_;
    std::string docRoot_;
};

}}
#ifndef  __YUX_HTTP_CLIENT_SOCKET_H_
#define  __YUX_HTTP_CLIENT_SOCKET_H_

#include "base/Socket.h"

namespace yux{
namespace common{

class HttpClientSocket : public yux::base::Socket
{
    public:
        HttpClientSocket() { }
        HttpClientSocket(const std::string& host, uint16_t port = 80) : yux::base::Socket(host.c_str(), port) { }
        int request(const std::string& url);
        int readCallBack(const char* buf, size_t size, SocketBase *sock);
};

}} //namespace

#endif

#ifndef  __YUX_HTTP_CLIENT_SOCKET_H_
#define  __YUX_HTTP_CLIENT_SOCKET_H_

#include "base/Socket.h"
#include "base/Fde.h"
#include "base/Singleton.h"

namespace yux{
namespace http{

class HttpClientSocket : public yux::base::Socket
{
    public:
        #define FDES yux::base::Singleton<yux::base::EpollFdes>
        HttpClientSocket() { FDES::getInstance()->create(); }
        HttpClientSocket(const std::string& host, uint16_t port = 80) : yux::base::Socket(host.c_str(), port) { FDES::getInstance()->create(); }
        int request(const std::string& url, CallBack cb);
        int readCallBack(const char* buf, size_t size, SocketBase *sock);
};

}} //namespace

#endif

#ifndef  __YUX_HTTP_CLIENT_SOCKET_H_
#define  __YUX_HTTP_CLIENT_SOCKET_H_

#include "base/Socket.h"

namespace yux{
namespace http{

class HttpClientSocket : public yux::base::Socket
{
    public:
        HttpClientSocket() { }
        HttpClientSocket(const std::string& host, uint16_t port = 80) : yux::base::Socket(host.c_str(), port) { }
        int request(const std::string& url, CallBack cb);
        int readCallBack(const char* buf, size_t size, SocketBase *sock);
        void setTimeout(uint32_t timeOut) { m_timeOut = timeOut; }
        void close();
    private:
        uint32_t m_timeOut;
};

}} //namespace

#endif

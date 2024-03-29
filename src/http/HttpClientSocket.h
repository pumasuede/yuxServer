#ifndef  __YUX_HTTP_CLIENT_SOCKET_H_
#define  __YUX_HTTP_CLIENT_SOCKET_H_

#include "base/Socket.h"

namespace yux{
namespace http{

class HttpClientSocket : public yux::base::Socket, public yux::base::SocketObserver
{
    public:
        HttpClientSocket() { addObserver(this); }
        HttpClientSocket(SocketObserver* pObserver) { addObserver(pObserver); }
        ~HttpClientSocket();

        void onReadEvent(SocketBase *sock, const char* buf, size_t size);
        void onCloseEvent(SocketBase *sock);

        int request(const std::string& url);
        void setTimeout(uint32_t timeOut) { m_timeOut = timeOut; }
    private:
        uint32_t m_timeOut;
};

}} //namespace

#endif

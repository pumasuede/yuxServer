#ifndef  __YUX_HTTP_CLIENT_SOCKET_H_
#define  __YUX_HTTP_CLIENT_SOCKET_H_

#include "base/Socket.h"

namespace yux{
namespace http{

class HttpClient : public yux::base::SocketObserver
{
    public:
        HttpClient() : socket_(this) { }
        HttpClient(SocketObserver* pObserver) : socket_(pObserver) { }
        ~HttpClient();

        yux::base::Socket* getSocket() { return &socket_; }
        void onReadEvent(yux::base::SocketBase *sock, const char* buf, size_t size);
        void onCloseEvent(yux::base::SocketBase *sock);

        int request(const std::string& url);
        void setTimeout(uint32_t timeOut) { m_timeOut = timeOut; }
    private:
        yux::base::Socket socket_;
        uint32_t m_timeOut;
};

}} //namespace

#endif

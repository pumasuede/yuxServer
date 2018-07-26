#ifndef  __YUX_HTTP_SERVER_SOCKET_H_
#define  __YUX_HTTP_SERVER_SOCKET_H_

#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "base/Thread.h"
#include "base/Socket.h"
#include "parser/HttpParser.h"

namespace yux{
namespace http{

typedef struct Req{
    uint32_t seq;
    int fd;
    std::string data;
} Req;

class HttpServerSocket : public yux::base::ServerSocket
{
public:
    static HttpServerSocket* create(const std::string& host, uint16_t port = 80);
    int readCallBack(const char* buf, size_t size, yux::base::SocketBase *sock);
    void setDocRoot(const std::string& docRoot);
    std::string getDocRoot() { return docRoot_; }
    std::list<Req>& getReqList() { return reqList_; }

private:
    HttpServerSocket(const std::string& host, uint16_t port) : ServerSocket(host.c_str(), port), docRoot_(".") {}
    std::string docRoot_;
    std::list<Req> reqList_;
};

class HttpServerThread : public yux::base::Thread
{
public:
    HttpServerThread(const std::string& name, HttpServerSocket *pServerSock) : yux::base::Thread(name), pServerSock_(pServerSock) {}
    void setServerSocket(HttpServerSocket* pServerSock) { pServerSock_ = pServerSock; }
    virtual void workBody();
    friend class HttpServerSocket;
private:
    HttpServerSocket* pServerSock_;
    yux::parser::HttpRequestParser httpParser_;
    static std::mutex mutex_;
    static std::condition_variable cv_;
};

}}
#endif

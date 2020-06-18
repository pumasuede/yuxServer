#ifndef  __YUX_HTTP_SERVER_SOCKET_H_
#define  __YUX_HTTP_SERVER_SOCKET_H_

#include <list>
#include <vector>
#include <fstream>

#include "base/Thread.h"
#include "base/Scheduler.h"
#include "base/Socket.h"
#include "parser/HttpParser.h"
#include "fastcgi/fcgi.h"

namespace yux{
namespace http{

typedef yux::base::SocketBase SocketBase;

struct Req{
    uint32_t seq;
    int fd;
    SocketBase* sock;
    std::string data;
};

struct RecvInfo
{
    RecvInfo() : bodyCnt(0), contentLength(0), hasBody(false) { }
    int bodyCnt; // Currently received body bytes;
    int contentLength;
    bool hasBody;
    std::string recvBuf;
};

class HttpService : public yux::base::SocketObserver
{

    public:
        static HttpService* create(const std::string& host, uint16_t port = 80);
        yux::base::ServerSocket* getServerSocket() { return &serverSock_; }
        void onReadEvent(SocketBase *sock, const char* buf, size_t size);

        void closeSocket(SocketBase *sock);

        std::string getDocRoot() { return docRoot_; }

        // http processer
        void handleRequest(void *pReq);
        void handleStatic(SocketBase *sock, yux::parser::HttpRequest& httpReq, std::ifstream& fs);
        void handleScript(SocketBase *sock, yux::parser::HttpRequest& httpReq);

    private:
        HttpService(const std::string& host, uint16_t port, const std::string& docRoot, yux::base::Scheduler *scheduler) :
                    serverSock_(host.c_str(), port, this), docRoot_(docRoot), scheduler_(scheduler) { }
        yux::base::ServerSocket serverSock_;
        yux::base::Scheduler *scheduler_;

        std::string docRoot_;

        std::map<SocketBase*, RecvInfo> recvInfoTable_;
        FastCgi  fastCgi_;
        yux::parser::HttpRequestParser httpParser_;
};

}}

#endif

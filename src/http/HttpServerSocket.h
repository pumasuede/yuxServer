#ifndef  __YUX_HTTP_SERVER_SOCKET_H_
#define  __YUX_HTTP_SERVER_SOCKET_H_

#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>

#include "base/Thread.h"
#include "base/Socket.h"
#include "parser/HttpParser.h"
#include "fastcgi/fcgi.h"

namespace yux{
namespace http{

struct Req{
    uint32_t seq;
    int fd;
    yux::base::SocketBase* sock;
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

class HttpServerSocket : public yux::base::ServerSocket
{
public:
    static HttpServerSocket* create(const std::string& host, uint16_t port = 80);
    int readCallBack(const char* buf, size_t size, yux::base::SocketBase *sock);
    void setDocRoot(const std::string& docRoot);
    void setMimeFile(const std::string& mimeFile);
    std::string getDocRoot() { return docRoot_; }
    std::string  getMimeFile() { return mimeFile_; }
    std::list<Req>& getReqList() { return reqList_; }
    void closeSocket(SocketBase* sock);

private:
    HttpServerSocket(const std::string& host, uint16_t port) : ServerSocket(host.c_str(), port) {}
    std::string docRoot_;
    std::string mimeFile_;
    std::list<Req> reqList_;
    yux::parser::HttpRequestParser httpParser_;
    std::map<SocketBase*, RecvInfo> recvInfoTable_;
};

class HttpWorkerThread : public yux::base::Thread
{
public:
    HttpWorkerThread(const std::string& name, HttpServerSocket *pServerSock) : yux::base::Thread(name), pServerSock_(pServerSock) { }
    void setServerSocket(HttpServerSocket* pServerSock) { pServerSock_ = pServerSock; }
    void handleStatic(yux::parser::HttpRequest& httpReq, std::ifstream& fs);
    void handleScript(yux::parser::HttpRequest& httpReq);
    virtual void workBody();
    friend class HttpServerSocket;
private:
    Req req_; // current socket request;
    FastCgi   fastCgi_;
    HttpServerSocket* pServerSock_;
    yux::parser::HttpRequestParser httpParser_;
    static std::mutex mutex_;
    static std::condition_variable cv_;
};

}}

#endif

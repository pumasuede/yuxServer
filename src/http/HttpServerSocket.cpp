#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "HttpServerSocket.h"
#include "base/Server.h"
#include "base/Log.h"

using namespace std;
using namespace yux::base;
using namespace yux::parser;

namespace yux{
namespace http{

std::mutex HttpServerThread::mutex_;
std::condition_variable HttpServerThread::cv_;

HttpServerSocket* HttpServerSocket::create(const string& host, uint16_t port)
{
    HttpServerSocket* pSock = new HttpServerSocket(host, port);
    pSock->setCbRead(std::bind(&HttpServerSocket::readCallBack, pSock, _1, _2, _3));
    return pSock;
}

void HttpServerSocket::setDocRoot(const string& docRoot)
{
    docRoot_ = docRoot;
}

int HttpServerSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    static string recvData;
    string tmpData(buf, size);
    log_debug("\n<RAW tmp data>:\n%s", tmpData.c_str());
    recvData += tmpData;
    if (tmpData.find("\r\n\r\n") == string::npos)
    {
        log_debug("Wait for more data");
        return 0;
    }

    static int n = 0;
    Req req;
    req.seq = n++;
    req.fd = sock->fd();
    req.data = recvData;
    log_debug("\n<Recv HTTP seq:%d fd:%d>:\n%s", req.seq, req.fd, recvData.c_str());
    if (recvData.size() > 0)
    {
        lock_guard<std::mutex> lock(HttpServerThread::mutex_);
        reqList_.push_back(req);
        HttpServerThread::cv_.notify_all();
    }

    recvData.clear();
}

void HttpServerThread::workBody()
{
    Req req;
    list<Req>& reqList = pServerSock_->getReqList();

    while (1)
    {
        {
            unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]{return reqList.size() > 0;});

            req = reqList.front();
            reqList.pop_front();
        }

        HttpRequest httpReq;

        bool parseRet = httpParser_.parse(httpReq, req.data.c_str(), req.data.size());
        log_debug("Process http request seq:%d %s\n", req.seq, httpReq.URI.c_str());

        if (!parseRet)
        {
            log_debug("parse http error");
            Server::getInstance().closeSocket(req.fd);
            continue;
        }

        // Check if the socket is still valid in case it's closed by remote peer.
        SocketBase *sock = Server::getInstance().getSocketByFd(req.fd);
        if (sock == nullptr)
        {
            log_debug("The socket on fd %d is already closed, nothing to do", sock->fd());
            return;
        }

        sock->sendStr("HTTP/1.1 200 OK\r\n");
        sock->sendStr("Connection: Kepp-Alive\r\n");
        sock->sendStr("Content-Type: text/html; charset=utf-8\r\n");
        sock->sendStr("Server: yuhttpd\r\n\r\n");

        string localFile = pServerSock_->getDocRoot() + httpReq.URI;

        string line;
        ifstream file(localFile.c_str());
        if (file.is_open())
        {
            while (getline(file,line))
            {
                sock->sendStr(line);
            }
            file.close();
        }
        else
        {
            sock->sendStr("Can't find Request URL "+httpReq.URI+" !<br>\n");
        }

        Server::getInstance().closeSocket(req.fd);
    }
}

}} // name space

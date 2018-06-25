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
namespace common{

HttpServerSocket* HttpServerSocket::create(const std::string& host, uint16_t port)
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
    string tmpData(buf, size);
    log_debug("\n[RAW HTTP]:%s\n", tmpData.c_str());
    recvData_ += string(buf, size);
    if (tmpData.find("\r\n\r\n") == string::npos)
    {
        log_debug("Wait for more data");
        return 0;
    }

    static int n = 0;
    HttpServerSocket* pServerSock = dynamic_cast<HttpServerSocket*>(sock);

    n++;
    HttpRequest req;
    bool parseRet = pServerSock->httpParser_.parse(req, recvData_.c_str(), recvData_.size());

    recvData_.clear();
    if (!parseRet)
    {
        return -1;
        log_debug("parse http error");
    }

    sock->sendStr("HTTP/1.1 200 OK\r\n");
    sock->sendStr("Connection: Kepp-Alive\r\n");
    sock->sendStr("Content-Type: text/html; charset=utf-8\r\n");
    sock->sendStr("Server: yuhttpd\r\n\r\n");

    string localFile = docRoot_ + req.URI;

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
        sock->sendStr("Can't find Request URL "+req.URI+" !<br>\n");
    }

    Server::getInstance().closeSocket(sock);
    return 0;
}

}}

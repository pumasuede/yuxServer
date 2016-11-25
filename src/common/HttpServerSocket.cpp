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

const string docRoot = ".";

namespace yux{
namespace common{

HttpServerSocket* HttpServerSocket::create(const char* host, uint16_t port)
{
     HttpServerSocket* pSock = new HttpServerSocket(host, port);
     pSock->setCbRead(std::tr1::bind(&HttpServerSocket::readCallBack, pSock, _1, _2, _3));
     return pSock;
}

int HttpServerSocket::readCallBack(char* buf, size_t size, SocketBase *sock)
{
    buf[size]=0;
    static int n = 0;
    HttpServerSocket* pServerSock = dynamic_cast<HttpServerSocket*>(sock);

    n++;
    HttpRequest req;
    if (pServerSock->httpParser_.parse(req, buf, size) == false)
    {
        return -1;
        cout<<"parse http error";
    }

    cout<<"[RAW HTTP]:\n"<<buf;
    sock->sendStr("HTTP/1.1 200 OK\r\n");
    sock->sendStr("Connection: Kepp-Alive\r\n");
    sock->sendStr("Content-Type: text/html; charset=ISO-8859-1\r\n");
    sock->sendStr("Server: Yux httpd\r\n\r\n");

    string localFile = docRoot + req.URI;

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

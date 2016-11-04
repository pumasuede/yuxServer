#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "HttpServer.h"
#include "base/Log.h"

using namespace std;
using namespace yux::base;
using namespace yux::parser;

namespace yux{
namespace common{

int HttpServer::readCallBack(char* buf, size_t size, SocketBase *sock, void *pArgs)
{
    buf[size]=0;
    static int n = 0;
    HttpServer* pServer = static_cast<HttpServer*>(pArgs);

    n++;
    HttpRequest req;
    if (pServer->httpParser_.parse(req, buf, size) == false)
    {
        cout<<"parse error";
    }

    cout<<"[RAW HTTP]:\n"<<buf;
    sock->sendStr("HTTP/1.1 200 OK\r\n");
    sock->sendStr("Connection: Kepp-Alive\r\n");
    sock->sendStr("Content-Type: text/html; charset=ISO-8859-1\r\n");
    sock->sendStr("Server: Yux httpd\r\n\r\n");
    sock->sendStr("<h1>Hello World</h1>");
    char line[1000];
    sprintf(line,"Count:%d <br>\n", n);
    sock->sendStr(line);
    sock->sendStr(buf);
    pServer->closeSocket(sock);
}

}}

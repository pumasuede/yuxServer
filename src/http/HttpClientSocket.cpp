#include <iostream>

#include "HttpClientSocket.h"
#include "base/Fde.h"
#include "base/Log.h"
#include "common/Utils.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

namespace yux{
namespace http{

HttpClientSocket::~HttpClientSocket()
{
    Fdes* fdes = FDES::getInstance();
    fdes->delWatch(fd_, Fde::READ);
}

void HttpClientSocket::onCloseEvent(SocketBase *sock)
{
    log_debug("Closing socket fd:%d", sock->fd());
}

void HttpClientSocket::onReadEvent(SocketBase *sock, const char* buf, size_t size)
{
    string data(buf, size);
    log_debug("HttpClientSocket::onReadEvent: %s", data.c_str());
}

int HttpClientSocket::request(const std::string& url)
{
    URLParser urlParser(url);
    urlParser.parse();
    remote_ = Peer(urlParser.host_.c_str(), urlParser.port_);

    Fdes* fdes = FDES::getInstance();
    fdes->addWatch(fd_, Fde::READ);

    int con = connect();
    if (con == -1)
    {
        log_debug("connect error: %s", strerror(errno));
        return -1;
    }

    string path = urlParser.path_.empty() ? "/" : urlParser.path_;
    string httpReq = string("GET ") + path + " HTTP/1.1\r\n";

    char hostLine[100] = {};
    sprintf(hostLine, "Host: %s:%d", urlParser.host_.c_str(), urlParser.port_);

    httpReq += hostLine;
    httpReq += "\r\nUser-Agent: YuxHttpClient\r\n";
    httpReq += "Connection: close\r\n";
    httpReq += "\r\n";

    log_debug("Sending request on fd:%d %s", fd_, hostLine);
    sendStr(httpReq);
    return 0;
}

}} //name space

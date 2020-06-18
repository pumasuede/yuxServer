#include <iostream>

#include "HttpClient.h"
#include "base/Fde.h"
#include "base/Log.h"
#include "common/Utils.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

namespace yux{
namespace http{

HttpClient::~HttpClient()
{
    Fdes* fdes = FDES::getInstance();
    fdes->delWatch(socket_.fd(), Fde::READ);
}

void HttpClient::onCloseEvent(SocketBase *sock)
{
    log_debug("Closing socket fd:%d", sock->fd());
}

void HttpClient::onReadEvent(SocketBase *sock, const char* buf, size_t size)
{
    string data(buf, size);
    log_debug("HttpClient::onReadEvent: %s", data.c_str());
}

int HttpClient::request(const std::string& url)
{
    URLParser urlParser(url);
    urlParser.parse();

    int con = socket_.connect(urlParser.host_.c_str(), urlParser.port_);
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

    log_debug("Sending request on fd:%d %s", socket_.fd(), hostLine);
    socket_.sendStr(httpReq);
    return 0;
}

}} //name space

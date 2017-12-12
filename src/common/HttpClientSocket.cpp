#include <iostream>
#include "HttpClientSocket.h"
#include "base/Utils.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace common{

int HttpClientSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    std::cout<<data<<"\n";
    return 0;
}

int HttpClientSocket::request(const std::string& url)
{

    URLParser urlParser(url);
    urlParser.parse();
    peer_ = Peer(urlParser.host_.c_str(), urlParser.port_);
    setCbRead(std::tr1::bind(&HttpClientSocket::readCallBack, this,  _1, _2, _3));
    int con = connect();
    if (con == -1)
    {
        cout<<"connect error"<<errno<<endl;
        return -1;
    }

    string httpReq = string("GET /") + urlParser.path_ + " HTTP/1.1\n";
    char hostLine[100] = {};
    sprintf(hostLine, "Host: %s:%d\n", urlParser.host_.c_str(), urlParser.port_);
    httpReq += hostLine;
    httpReq += "User-Agent: YuxHttpClient \n";
    httpReq += "\r\n\r\n";

    sendStr(httpReq);
    read();
}

}} //name space

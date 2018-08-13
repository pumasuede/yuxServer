#include <iostream>
#include "HttpClientSocket.h"
#include "base/Utils.h"
#include "base/Log.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace http{

void HttpClientSocket::close()
{
    Fdes* fdes = FDES::getInstance();
    fdes->delWatch(fd_, Fde::READ);
    ::close(fd_);
}

int HttpClientSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    log_debug("HttpClientSocket::readCallBack: %s", data.c_str());
    return 0;
}

int HttpClientSocket::request(const std::string& url, CallBack cb)
{
    URLParser urlParser(url);
    urlParser.parse();
    peer_ = Peer(urlParser.host_.c_str(), urlParser.port_);
    setCbRead(cb);

    Fdes* fdes = FDES::getInstance();
    fdes->addWatch(fd_, Fde::READ);

    int con = connect();
    if (con == -1)
    {
        cout<<"connect error:"<<errno<<endl;
        return -1;
    }

    string httpReq = string("GET /") + urlParser.path_ + " HTTP/1.1\n";
    char hostLine[100] = {};
    sprintf(hostLine, "Host: %s:%d\n", urlParser.host_.c_str(), urlParser.port_);
    httpReq += hostLine;
    httpReq += "User-Agent: YuxHttpClient \n";
    httpReq += "Connection: close \n";
    httpReq += "\r\n\r\n";

    sendStr(httpReq);

    while (1)
    {
        int n = fdes->wait(m_timeOut);

        if (n<0)
            return -1;

        if (n==0)
        {
            //timeout
            log_debug("http client request timeout");
            return -1;
        }

        vector<Fde*>& readyFdes = fdes->readyList();
        if (n != readyFdes.size())
        {
            cout<<"Warn: wait result doesn't match ready Fd events "<<n<<" : "<<readyFdes.size()<<" \n";
        }

        for (int i=0; i<readyFdes.size(); i++)
        {
            Fde *fde = readyFdes[i];
            int fd = fde->fd();

            if (fd != fd_ || !fde->readable())
                continue;

            int ret = read();

            if (ret == 0)
            {
                log_debug("Socket is closed by peer, closing socket - Fd: %d", fd);
                close();
                return -1;
            }

            if (ret < 0)
            {
                close();
                cout<<"Socket read error, will delete socket - Fd: "<<fd <<"\n";
                return -1;
            }
        }
    }
}

}} //name space

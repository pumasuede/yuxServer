#include <iostream>
#include "HttpClientSocket.h"
#include "base/Utils.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace http{

int HttpClientSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    std::cout<<data<<"\n";
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
        int n = fdes->wait();

        if (n<0)
            return -1;

        if (n==0)
        {
            //timeout
            cout<<"timeout\n";
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

            if (fd != fd_)
                continue;

            if (fde->readable())
            {
                int ret = read();
                if (ret == 0)
                {
                    cout<<"Socket is closed by peer, closing socket - Fd: "<<fd <<"\n";
                    close();
                    return -1;
                }
                if (ret < 0)
                {
                    cout<<"Socket read error, will delete socket - Fd: "<<fd <<"\n";
                    close();
                    return -1;
                }
            }
        }
    }
}

}} //name space

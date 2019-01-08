#include <iostream>
#include "base/Fde.h"
#include "base/Log.h"
#include "http/HttpClientSocket.h"

using namespace std;
using namespace yux::base;
using namespace yux::http;

int readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    //std::cout<<"received "<<size<<" bytes in call back\n";
    std::cout<<data<<"\n";
    return 0;
}

void timerCallBack(void*)
{
    cout<<"timerCallBack";
}

int testHttpClient(const char *url)
{
    HttpClientSocket httpClient;
    httpClient.request(url, std::bind(&readCallBack, _1, _2, _3));

    Fdes *fdes = FDES::getInstance();

    while (1)
    {
        int n = fdes->wait();

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

        for (auto fde : readyFdes)
        {
            int fd = fde->fd();

            if (fd != httpClient.fd() || !fde->readable())
                continue;

            int ret = httpClient.read();

            if (ret == 0)
            {
                log_debug("Socket is closed by peer, closing socket - Fd: %d", fd);
                httpClient.close();
                return -1;
            }

            if (ret < 0)
            {
                httpClient.close();
                cout<<"Socket read error, will delete socket - Fd: "<<fd <<"\n";
                return -1;
            }
        }
    }
}

int main(int argc, char** argv)
{
    log_open("test.log");
    const char *url = argc > 1 ? argv[1] : "baidu.com";

    return testHttpClient(url);
}

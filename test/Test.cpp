#include <iostream>
#include "base/Socket.h"
#include "common/HttpClientSocket.h"
#include "base/Fde.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

int readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    std::cout<<data<<"\n";
    return 0;
}

void timerCallBack(void*)
{
    cout<<"timerCallBack";
}

void testHttpClient(const char *url)
{
    HttpClientSocket httpClient;
    httpClient.request(url);
}

int testSocket()
{
    Socket sock;
    sock.setCbRead(std::tr1::bind(&readCallBack, _1, _2, _3));

    //Fdes *fdes = new SelectFdes;
    Fdes *fdes = new EpollFdes;
    fdes->create();
    fdes->addWatch(sock.fd(), Fde::READ);
    //fdes->addWatch(sock.fd(), Fde::WRITE);

    const char* host = "baidu.com";
    int con = sock.connect(host, 80);
    if (con == -1)
    {
        cout<<"connect error"<<errno<<endl;
        exit errno;
    }

    while (1)
    {
        int n = fdes->wait(50);

        if (n<0)
            return -1;

        if (n==0)
        {
            cout<<"Timeout in wait\n";
            int ret = sock.sendStr("GET / HTTP/1.1\n\r\n\r\n");
            if (ret == -1)
            {
                cout<<"send error"<<errno<<endl;
            }
            continue;
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

            if (fd != sock.fd())
                continue;

            /*
               if (fde->writable())
               {
               cout<<"fde "<<fde->fd() <<" writable is true. Send reuqest\n";
               int ret = sock.sendStr("GET / HTTP/1.1\n\r\n\r\n");
               if (ret == -1)
               {
               cout<<"send error"<<errno<<endl;
               }
               }
             */

            if (fde->readable())
            {
                int ret = sock.read();
                if (ret < 0)
                {
                    cout<<"Socket read error, will delete socket - Fd: "<<fd <<"\n";
                    sock.close();
                    exit(0);
                }
            }
        }
    }

}

int main(int argc, char** argv)
{
    const char *url = argc > 1 ? argv[1] : "baidu.com";
    testHttpClient(url);

    return 0;
}

#include <iostream>
#include "base/Socket.h"
#include "base/Fde.h"

using namespace std;
using namespace yux::base;

int readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    std::cout<<data<<"\n";
    return 0;
}

int main(int argc, char** argv)
{
    Socket sock;
    sock.setCbRead(std::tr1::bind(&readCallBack, _1, _2, _3));

    Fdes *fdes = new SelectFdes;
    fdes->addWatch(sock.fd(), Fde::READ);

    sock.connect("10.19.228.56", 80);
    while (1)
    {
        int n = fdes->wait();

        if (n<=0)
            return -1;

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

            if (fde->readable())
            {
                int ret = sock.read();
                if (ret < 0)
                {
                    cout<<"abnormal in Socket read, will delete socket - Fd: "<<fd <<"\n";
                    sock.close();
                }
            }

        }
    }
}

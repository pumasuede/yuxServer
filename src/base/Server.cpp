#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "Server.h"
#include "Log.h"
#include "Fde.h"

using namespace std;

namespace yux{
namespace base{

int Server::readCallBack(char* buf, size_t size, SocketBase *sock, void *pArgs)
{
    buf[size] = 0;
    cout<<"read "<<size<<" bytes:"<<buf<<endl;
    log_debug("read %d bytes - [%s]", size, buf);
    return 1;
}

Server::Server(std::string host, uint16_t port, SocketBase::CbFun cbRead)
: cbRead_(cbRead)
{
    stop_ = false;
    rlimit r;
    if (-1 == getrlimit( RLIMIT_NOFILE, &r ))
        return;
    uint32_t fdMax = r.rlim_max < 65535 && r.rlim_max > 0 ? r.rlim_max : 65535;
    log_debug("fdMax:%d", fdMax);

    // Initialize all IOBaseBase * to NULL
    fdToSkt_.resize(fdMax);
    for ( int n = 0; n < fdMax; ++n )
        fdToSkt_[n] = NULL;

    // create socket
    servSock_ = new ServerSocket(host.c_str(), port);
    if (servSock_->listen() == -1)
    {
        log_debug("Error at listening errno: %d", errno);
        cout << "Error at listening errno:"<<errno<<"\n";
        exit(-1);
    }

    int servFd = servSock_->fd();
    log_debug("listen on IP: %s and Port: %d Fd: %d", servSock_->getPeer().host_.c_str(), servSock_->getPeer().port_, servFd);
    cout<<"listen on IP:"<<servSock_->getPeer().host_<<" and Port:"<<servSock_->getPeer().port_<<" Fd:"<<servFd<<"...\n";

    // create events;
    #ifdef __linux__
        fdes_ = new EpollFdes();
    #else
        fdes_ = new SelectFdes();
    #endif

    fdes_->create();
    fdToSkt_[servFd] = servSock_;
    fdes_->addWatch(servFd, Fde::READ);
}

Server::~Server()
{
    servSock_->close();
    delete fdes_;
}

void Server::loopOnce()
{
    int n = fdes_->wait();
    if (n<=0)
        return;
    vector<Fde*>& readyFdes = fdes_->readyList();
    if (n != readyFdes.size())
    {
        cout<<"Warn: wait result doesn't match ready Fd events "<<n<<" : "<<readyFdes.size()<<" \n";
    }

    for (int i=0; i<readyFdes.size(); i++)
    {
        Fde *fde = readyFdes[i];
        int fd = fde->fd();
        SocketBase *skt = fdToSkt_[fd];
        if (fde->readable())
        {
            if (fd == servSock_->fd())
            {
                SocketBase *newSkt = dynamic_cast<ServerSocket*>(skt)->accept();
                int newFd = newSkt->fd();
                fdToSkt_[newFd] = newSkt;
                fdes_->addWatch(newFd, Fde::READ);
                newSkt->setCbRead(cbRead_, (void*)this);
                cout<<"ret new socket: "<<newSkt<<" Fd:"<<newFd<<"...\n";
            }
            else
            {
                cout<<"read on client socket "<<fd<<" \n";
                int ret = skt->read();
                if (ret < 0)
                {
                    cout<<"abnormal in Socket read, will delete socket..\n";
                    closeSocket(skt);
                }
            }
        }
        else if (fde->writable())
        {
            skt->write();
            fdes_->delWatch(fd, Fde::WRITE);
        }
    }
}

void Server::closeSocket(SocketBase* sock)
{
    int fd = sock->fd();
    fdes_->delWatch(fd, Fde::READ);
    fdes_->delWatch(fd, Fde::WRITE);
    fdToSkt_[fd] = NULL;
    delete sock;
}

void Server::loop()
{
    while (!stop_)
    {
        loopOnce();
    }
}

int Server::getListenFd()
{
    return servSock_->fd();
}

}}

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "Server.h"
#include "../base/Socket.h"
#include "../base/Fde.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace common{

Server::Server(string host, uint16_t port)
{
    rlimit r;
    if (-1 == getrlimit( RLIMIT_NOFILE, &r ))
        return;
    uint32_t fdMax = r.rlim_max;

    // Initialize all IOBaseBase * to NULL
    fdToSkt_.resize(fdMax);
    for ( int n = 0; n < fdMax; ++n )
        fdToSkt_[n] = NULL;


    // create socket
    servSock_ = new ServerSocket(host.c_str(), port);
    if (servSock_->listen() == -1)
    {
        cout << "Error at listening errno:"<<errno<<"\n";
        exit(-1);
    }

    int servFd = servSock_->fd();
    cout<<"listen on IP:"<<servSock_->getPeer().host_<<" and Port:"<<servSock_->getPeer().port_<<" Fd:"<<servFd<<"...\n";

    // create events;
    fdes_ = new EpollFdes();
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
                cout<<"ret new socket: "<<newSkt<<" listenFd:"<<newFd<<"...\n";
            }
            else
            {
                cout<<"read on client socket "<<fd<<" \n";
                int ret = skt->read();
                if (ret < 0)
                {
                    cout<<"abnormal in Socket read, will delete socket..\n";
                    fdes_->delWatch(fd, Fde::READ);
                    fdToSkt_[fd] = NULL;
                    delete skt;
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

int Server::getListenFd()
{
    return servSock_->fd();
}


}}

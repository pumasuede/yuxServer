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

Server& Server::getInstance()
{
    static Server server;
    return server;
}

void Server::init(std::string host, uint16_t port, SocketBase::CallBack cbRead, Timer* timer)
{
    // Create socket
    init();
    ServerSocket* defaultServerSock = new ServerSocket(host.c_str(), port, cbRead);
    addServerSocket(defaultServerSock);
    addTimer(timer);
}

void Server::init()
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

    // create events;
    #ifdef __linux__
        fdes_ = new EpollFdes();
    #else
        fdes_ = new SelectFdes();
    #endif

    fdes_->create();
}

void Server::addTimer(Timer* timer)
{
    if (timer == NULL)
        return;
    timers_.push_back(timer);
}

void Server::addTimer(int intval, Timer::TimerCallBack timerCb)
{
    Timer *timer = new Timer(intval, timerCb);
    timers_.push_back(timer);
}

void Server::addServerSocket(SocketBase* pServerSocket)
{
    int servFd = pServerSocket->fd();
    const Peer& peer = pServerSocket->getPeer();
    log_debug("listen on IP: %s and Port: %d Fd: %d", peer.host_.c_str(), peer.port_, servFd);
    cout<<"listen on IP:"<<peer.host_<<" and Port:"<<peer.port_<<" Fd:"<<servFd<<"...\n";

    fdToSkt_[servFd] = pServerSocket;
    fdes_->addWatch(servFd, Fde::READ);
    servSockList_.insert(pServerSocket);
}

Server::~Server()
{
    std::set<SocketBase*>::iterator it = servSockList_.begin();
    for (; it!=servSockList_.end(); ++it)
    {
        delete *it;
    }

    for (int i=0; i<fdToSkt_.size(); i++)
    {
        delete fdToSkt_[i];
    }

    delete fdes_;
}

void Server::loopOnce()
{
    // Compute timer
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int current = tv.tv_sec*1000 + tv.tv_usec/1000;
    int min = INT_MAX;
    for (auto it : timers_)
    {
        if (it->lastFired_ == -1)
        {
            it->lastFired_ = current;
        }

        int nextFire = it->lastFired_ + it->mSec_;
        if (nextFire > current && nextFire < min)
        {
            min = nextFire;
        }
    }

    int n = fdes_->wait(min - current);

    // Error!
    if (n<0)
        return;

    // Timeout, lauch timer call back
    if (n==0)
    {
        gettimeofday(&tv, NULL);
        int current = tv.tv_sec*1000 + tv.tv_usec/1000;
        for (auto it : timers_)
        {
            it->timerCb_(this);
            it->lastFired_ = current;
        }
    }

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
        if (!skt)
            continue;
        if (fde->readable())
        {
            if (servSockList_.find(skt) != servSockList_.end())
            {
                SocketBase *newSkt = dynamic_cast<ServerSocket*>(skt)->accept();
                int newFd = newSkt->fd();
                fdToSkt_[newFd] = newSkt;
                fdes_->addWatch(newFd, Fde::READ);
                cout<<"accept new socket - Fd:"<<newFd<<"...\n";
            }
            else
            {
                cout<<"read on client socket "<<fd<<" \n";
                int ret = skt->read();
                if (ret < 0)
                {
                    cout<<"abnormal in Socket read, will delete socket - Fd: "<<fd <<"\n";
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
    cout<<"deleting socket - Fd: "<<fd <<"\n";
    delete sock;
}

void Server::loop()
{
    while (!stop_)
    {
        loopOnce();
    }
}

}}

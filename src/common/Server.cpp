#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <iostream>

#include "Server.h"
#include "base/Log.h"
#include "base/Fde.h"

using namespace std;
using namespace yux::base;

namespace yux{
namespace common{

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

    // create FDES
    fdes_ = FDES::getInstance();

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
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


Timer* Server::getMinTimer(int current)
{
    int min = INT_MAX;
    Timer* pTimer = nullptr;

    for (auto timer : timers_)
    {
        if (timer->lastFired_ == -1)
        {
            timer->lastFired_ = current;
        }

        int toFire = timer->lastFired_ + timer->mSec_;

        if (toFire < min)
        {
            min = toFire;
            pTimer = timer;
        }
    }

    return pTimer;
}

void Server::addServerSocket(SocketBase* pServerSocket)
{
    int servFd = pServerSocket->fd();
    const Peer& local = pServerSocket->getLocal();
    log_info("listen on IP: %s and Port: %d Fd: %d", local.host_.c_str(), local.port_, servFd);
    cout<<"listen on IP:"<<local.host_<<" and Port:"<<local.port_<<" Fd:"<<servFd<<"...\n";

    regSocket(pServerSocket);
    servSockList_.insert(pServerSocket);
}

Server::~Server()
{
    for (auto sock : servSockList_)
    {
        delete sock;
    }

    delete fdes_;
}

void Server::loopOnce()
{
    // Compute timer
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int current = tv.tv_sec*1000 + tv.tv_usec/1000;
    Timer* pTimer = getMinTimer(current);

    int timeToFire = 0;

    if (pTimer)
    {
        int toFire = pTimer->lastFired_ + pTimer->mSec_;
        int gap = toFire - current;
        //log_trace("nextFire/current time point: %d-%d gap=%dms", toFire, current, gap );
        int timeToFire = gap > 0 ? gap : 0;
    }

    int n = fdes_->wait(timeToFire);

    // Error!
    if (n < 0)
    {
        log_error("Fdes wait returns error %s", strerror(errno) );
        return;
    }

    // Timeout, lauch timer call back
    if (n == 0)
    {
        gettimeofday(&tv, NULL);
        int current = tv.tv_sec*1000 + tv.tv_usec/1000;

        for (auto timer : timers_)
        {
            if (timer->lastFired_ + timer ->mSec_ < current+10)
            {
                timer->timerCb_(this);
                timer->lastFired_ = current;
            }
        }
    }

    vector<Fde*>& readyFdes = fdes_->readyList();

    for (auto fde : readyFdes)
    {
        int fd = fde->fd();

        shared_ptr<SocketBase> skt;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            skt = fdToSkt_[fd];
        }

        if (!skt)
        {
            log_debug("Socket is closed after fde wait, Fd: %d", fd);
            continue;
        }


        if (fde->readable())
        {
            if (servSockList_.find(skt.get()) != servSockList_.end())
            {
                SocketBase *newSkt = dynamic_cast<ServerSocket*>(skt.get())->accept();
                regSocket(newSkt);
            }
            else
            {
                int ret = skt->read();
                if (ret == 0)
                {
                    log_debug("Socket is closed by peer, closing socket - Fd: %d", fd);
                    closeSocket(skt.get());
                }
                if (ret < 0)
                {
                    log_debug("Abnormal in Socket read, will delete socket - Fd: %d", fd);
                    closeSocket(skt.get());
                }
            }
        }
        else if (fde->writable())
        {
            // ignore it for now.
        }
    }
}

void Server::regSocket(SocketBase* pSocket)
{
    int fd = pSocket->fd();

    std::lock_guard<std::mutex> lock(mutex_);

    fdes_->addWatch(fd, Fde::READ);
    fdes_->addWatch(fd, Fde::WRITE);

    fdToSkt_[fd].reset(pSocket);
}

void Server::closeSocket(SocketBase* pSocket)
{
    int fd = pSocket->fd();
    log_debug("Closing socket - Fd:%d ", fd);

    std::lock_guard<std::mutex> lock(mutex_);

    fdes_->delWatch(fd, Fde::READ);
    fdes_->delWatch(fd, Fde::WRITE);

    fdToSkt_[fd].reset();
}

void Server::loop()
{
    while (!stop_)
    {
        loopOnce();
    }
}

}} // name space

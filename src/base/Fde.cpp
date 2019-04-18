#include <iostream>
#include <errno.h>
#include <string.h>

#include "Fde.h"
#include "base/Log.h"

using namespace std;

namespace yux {
namespace base {

Fde* Fdes::getFde(int fd)
{
    while (fdeList_.size() <= fd)
    {
        Fde *fde = new Fde(fdeList_.size(), Fde::NONE, Fde::NONE);
        fdeList_.push_back(fde);
    }
    return fdeList_[fd];
}

Fdes::~Fdes()
{
    std::vector<Fde*>::iterator it = fdeList_.begin();
    int fdMax = 0;
    while (it != fdeList_.end())
    {
        delete *it;
        ++it;
    }
    fdeList_.clear();
    readyList_.clear();
}

// Select

void SelectFdes::init()
{
    if (!init_)
    {
        FD_ZERO(&rfds_);
        FD_ZERO(&wfds_);
        FD_ZERO(&efds_);
        maxFd_ = 0;
        init_ = true;
    }
}

void SelectFdes::addWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    int watchEvents = fde->watchEvents();
    watchEvents |= event;

    if (watchEvents & Fde::READ)
    {
        FD_SET(fd, &rfds_);
    }
    if (watchEvents & Fde::WRITE)
    {
        FD_SET(fd, &wfds_);
    }

    if (fd > maxFd_)
    {
        maxFd_ = fd;
    }

    fde->setWatchEvents(watchEvents);
}

void SelectFdes::delWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    int watchEvents = fde->watchEvents();
    watchEvents &= ~event;

    if (!(watchEvents & Fde::READ))
    {
        FD_CLR(fd, &rfds_);
    }

    if (!(watchEvents & Fde::WRITE))
    {
        FD_CLR(fd, &wfds_);
    }

    int n = fdeList_.size()-1;
    while (n>0 && fdeList_[n]->events() == Fde::NONE)
    {
        --n;
    }

    maxFd_ = n;
    fde->setWatchEvents(watchEvents);
}

int SelectFdes::wait(int mSecTimeout)
{
    fd_set rfds = rfds_;
    fd_set wfds = wfds_;
    fd_set efds = efds_;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = mSecTimeout*1000;
    int n = select(maxFd_+1, &rfds, &wfds, &efds, &tv);

    if (n<0)
    {
        std::cout<<"Error at select: "<<strerror(errno)<<"\n";
        return n;
    }

    Fde *fde;
    int fdEvents;

    readyList_.clear();

    for (int i = 0; i <= maxFd_; i++)
    {
        fdEvents = Fde::NONE;
        if ( FD_ISSET(i, &rfds) )
            fdEvents |= Fde::READ;
        if ( FD_ISSET(i, &wfds) )
            fdEvents |= Fde::WRITE;
        if ( FD_ISSET(i, &efds) )
            fdEvents |= Fde::EXCEPT;

        if (fdEvents == Fde::NONE)
            continue;

        fde = getFde(i);
        fde->setEvents(fdEvents);
        readyList_.push_back(fde);
    }

    // log_trace("ready events num: %d  fd num: %d", n, readyList_.size());
    return n;
}

// Epoll
#ifdef __linux__

void EpollFdes::init()
{
    if (!init_)
    {
        events_ = new struct epoll_event[ee_size_];
        epollFd_ = epoll_create(10);
        init_ = true;
    }
}

void EpollFdes::addWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    int watchEvents = fde->watchEvents();
    int op = watchEvents == Fde::NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    watchEvents |= event;
    struct epoll_event ee = {0};
    ee.events = 0;

    if (watchEvents & Fde::READ)
    {
         ee.events |= EPOLLIN;
    }
    if (watchEvents & Fde::WRITE)
    {
        ee.events |= EPOLLOUT;
    }
    ee.data.fd  = fd;
    epoll_ctl(epollFd_, op, fd, &ee);

    fde->setWatchEvents(watchEvents);
}

void EpollFdes::delWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    int watchEvents = fde->watchEvents();
    watchEvents &= ~event;
    int op = watchEvents == Fde::NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    struct epoll_event ee = { 0 };
    if (event & Fde::READ)
    {
        ee.events |= EPOLLIN;
    }
    if (event & Fde::WRITE)
    {
        ee.events |= EPOLLOUT;
    }
    ee.data.fd  = fd;

    epoll_ctl(epollFd_, op, fd, &ee);
    fde->setWatchEvents(watchEvents);
}

int EpollFdes::wait(int mSecTimeout)
{
    int n = epoll_wait(epollFd_, events_, ee_size_, mSecTimeout);
    if (n<0)
        std::cout<<"Error at epoll_wait: "<<strerror(errno)<<"\n";

    readyList_.clear();

    for (int i = 0; i < n; i++)
    {
        int fd = events_[i].data.fd;
        uint32_t events = events_[i].events;
        int fdEvents = Fde::NONE;

        if (events & EPOLLIN)
        {
            fdEvents |= Fde::READ;
        }

        if (events & EPOLLOUT)
        {
            fdEvents |= Fde::WRITE;
        }

        Fde *fde = getFde(fd);
        fde->setEvents(fdEvents);
        readyList_.push_back(fde);
    }

    // log_trace("ready events num: %d  fd num: %d", n, readyList_.size());
    return n;
}
#endif

}}

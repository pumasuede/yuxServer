#include <iostream>
#include <errno.h>
#include "Fde.h"

namespace yux {
namespace base {


Fde* Fdes::getFde(int fd)
{
    while (fdeList_.size() <= fd)
    {
        Fde *fde = new Fde(fdeList_.size(), Fde::NONE);
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

void SelectFdes::create()
{
    FD_ZERO(&rfds_);
    FD_ZERO(&wfds_);
    FD_ZERO(&efds_);
    maxFd_ = 0;
}

void SelectFdes::addWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    fde->setEvents(fde->events()|event);

    if (event == Fde::READ)
    {
        FD_SET(fd, &rfds_);
    }
    if (event == Fde::WRITE)
    {
        FD_SET(fd, &wfds_);
    }

    if (fd > maxFd_)
    {
        maxFd_ = fd;
    }
}

void SelectFdes::delWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    fde->setEvents(Fde::NONE);

    if (event == Fde::READ)
    {
        FD_CLR(fd, &rfds_);
    }

    if (event == Fde::WRITE)
    {
        FD_CLR(fd, &wfds_);
    }

    int n = fdeList_.size()-1;
    while (n>0 && fdeList_[n]->events() == Fde::NONE)
    {
        --n;
    }

    maxFd_ = n;
}

int SelectFdes::wait()
{
    fd_set rfds = rfds_;
    fd_set wfds = wfds_;
    fd_set efds = efds_;

    int n = select(maxFd_+1, &rfds, &wfds, &efds, 0);
    if (n<0)
        std::cout<<"Error at select: "<<errno<<"\n";

    Fde *fde;
    int fdEvents;

    readyList_.clear();

    for (int i = 0; i<= maxFd_; i++)
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

    return n;
}
//Epoll
#ifdef __linux__

EpollFdes::EpollFdes():ee_size_(100)
{
    events_ = new struct epoll_event[ee_size_];
}

void EpollFdes::create()
{
    epollFd_ = epoll_create(10);
}

void EpollFdes::addWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    fde->setEvents(Fde::NONE);

    struct epoll_event ee = { 0 };
    ee.events = (event = Fde::READ) ? EPOLLIN : EPOLLOUT;
    ee.data.fd  = fd;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ee);
}

void EpollFdes::delWatch(int fd, Fde::FdEvent event)
{
    Fde *fde = getFde(fd);
    fde->setEvents(Fde::NONE);

    struct epoll_event ee = { 0 };
    ee.events = (event == Fde::READ) ? EPOLLIN : EPOLLOUT;
    ee.data.fd  = fd;

    epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ee);
}

int EpollFdes::wait()
{
    int n = epoll_wait(epollFd_, events_, ee_size_, -1);
    if (n<0)
        std::cout<<"Error at epoll_wait: "<<errno<<"\n";

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

    return n;
}
#endif

}}

#include <iostream>
#include <errno.h>
#include "Fde.h"

namespace yux {
namespace base {

// Select

bool SelectFde::readable()
{
    return true;
}

bool SelectFde::writable()
{
    return true;
}

// Epoll

bool EpollFde::readable()
{
    return events_ & EPOLLIN;
}

bool EpollFde::writable()
{
    return events_ & EPOLLOUT;
}

EpollFdes::EpollFdes():ee_size_(100)
{
    events_ = new struct epoll_event[ee_size_];
}

void EpollFdes::create()
{
    epollFd_ = epoll_create(10);
}

void EpollFdes::addWatch(int fd, Fdes::FdEvent event)
{
    struct epoll_event ee = { 0 };
    ee.events = (event == READ) ? EPOLLIN : EPOLLOUT;
    ee.data.fd  = fd;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ee);
}

void EpollFdes::delWatch(int fd, Fdes::FdEvent event)
{
    struct epoll_event ee = { 0 };
    ee.events = (event == READ) ? EPOLLIN : EPOLLOUT;
    ee.data.fd  = fd;

    epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ee);
}

int EpollFdes::wait()
{
    int n = epoll_wait(epollFd_, events_, ee_size_, -1);
    if (n<0)
        std::cout<<"Error at epoll_wait: "<<errno<<"\n";

    for (int i = 0; i < n; i++)
    {
        int fd = events_[i].data.fd;
        uint32_t events = events_[i].events;
        Fde *fde = new EpollFde(fd, events);
        fdeList_.push_back(fde);
    }

    return n;
}

}}

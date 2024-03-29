#ifndef __YUX_FDE_H__
#define __YUX_FDE_H__

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <sys/select.h>
#include <vector>

#include "Singleton.h"

namespace yux {
namespace base {

class Fde
{
    public:
        typedef enum
        {
            NO_TYPE   = 0,
            SELECT = 1,
            EPOLL  = 2
        } FdeType;

        typedef enum
        {
            NONE = 0,
            READ = 1,
            WRITE = 1<<1,
            EXCEPT = 1<<2
        } FdEvent;

        Fde() : fd_(0), events_(NONE), watchEvents_(NONE) {}
        Fde(int fd, int events, int watchEvents) : fd_(fd), events_(events), watchEvents_(watchEvents) {}
        virtual bool readable() { return events_ & READ; }
        virtual bool writable() { return events_ & WRITE; }
        virtual bool isExcept() { return events_ & EXCEPT; }
        virtual int fd() { return fd_; }
        virtual void setEvents(int events) { events_ = events; }
        virtual int events() { return events_; }
        virtual int watchEvents() { return watchEvents_; }
        virtual void setWatchEvents(int events) { watchEvents_ = events; }
    private:
        int events_;
        int watchEvents_;
        int fd_;
};

class Fdes
{
    public:
        Fde* getFde(int fd);
        Fdes() : init_(false) {}
        virtual ~Fdes();
        // init fdes.
        virtual void init() = 0;
        // return the count of events to be handled.
        virtual int wait(int mSecTimeout =-1) = 0;
        // add fd to watch
        virtual void addWatch(int fd, Fde::FdEvent event) = 0;
        virtual void delWatch(int fd, Fde::FdEvent event) = 0;

        // get available fdes;
        std::vector<Fde*>& readyList() { return readyList_; }
        std::vector<Fde*>& fdeList() { return fdeList_; }

    protected:
        std::vector<Fde*> readyList_;
        std::vector<Fde*> fdeList_;
        bool init_;
};

// Select

class SelectFdes : public Fdes, public Singleton<SelectFdes>
{
    friend class Singleton<SelectFdes>;
    public:
        void init();
        int wait(int mSecTimeout);
        void addWatch(int fd, Fde::FdEvent event);
        void delWatch(int fd, Fde::FdEvent event);
        Fde::FdeType type() { return Fde::SELECT; }
    private:
        SelectFdes() { init(); }
        ~SelectFdes() {}

        fd_set rfds_;
        fd_set wfds_;
        fd_set efds_;
        int    maxFd_;
};


// Epoll

#ifdef __linux__
class EpollFdes : public Fdes, public Singleton<EpollFdes>
{
    friend class Singleton<EpollFdes>;
    public:
        void init();
        int wait(int mSecTimeout);
        void addWatch(int fd, Fde::FdEvent event);
        void delWatch(int fd, Fde::FdEvent event);
        Fde::FdeType type() { return Fde::EPOLL; }
    private:
        EpollFdes(): ee_size_(100), epollFd_(0) { init(); }
        ~EpollFdes() { delete[] epollEvents_; }

        int epollFd_;
        const int ee_size_;
        // epoll events
        struct epoll_event *epollEvents_;
};
    #define FDES EpollFdes
#else
    #define FDES SelectFdes
#endif

}} //namespace
#endif

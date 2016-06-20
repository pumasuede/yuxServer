#include <sys/epoll.h>
#include <vector>

namespace yux {
namespace base {

class Fde
{
    public:
        typedef enum
        {
            NONE   = 0,
            SELECT = 1,
            EPOLL  = 2
        } FdeType;


        Fde() : fd_(0) {}
        Fde(int fd) : fd_(fd) {}
        virtual bool readable() { return false; }
        virtual bool writable() { return false; }
        virtual FdeType type()  { return NONE; }
        virtual int fd() { return fd_; }
    private:
        int fd_;
};

class Fdes
{

    public:
        typedef enum
        {
            READ = 0,
            WRITE =1
        } FdEvent;

        // init fdes.
        virtual void create() = 0;
        // return the count of events to be handled.
        virtual int wait() = 0;
        // add fd to watch
        virtual void addWatch(int fd, Fdes::FdEvent event) = 0;
        virtual void delWatch(int fd, Fdes::FdEvent event) = 0;

        // get available fdes;
        std::vector<Fde*>& fdeList() { return fdeList_; }

    protected:
        std::vector<Fde*> fdeList_;
};

// Select

class SelectFde: public Fde
{
    public:
        bool readable();
        bool writable();
        Fde::FdeType type() { return Fde::SELECT; }
    private:
        int events_;
};

// Epoll

class EpollFde : public Fde
{
    public:
        EpollFde(int fd, uint32_t events) : Fde(fd), events_(events){}
        bool readable();
        bool writable();
        Fde::FdeType type() { return Fde::EPOLL; }
    private:
        uint32_t  events_;
};

class EpollFdes : public Fdes
{
    public:
        EpollFdes();
        void create();
        int wait();
        void addWatch(int fd, Fdes::FdEvent event);
        void delWatch(int fd, Fdes::FdEvent event);
    private:
        int epollFd_;
        const int ee_size_;
        // epoll events
        struct epoll_event *events_;
};

}}

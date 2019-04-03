#ifndef __YUX_SOCKET_H__
#define __YUX_SOCKET_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>

#include <string>
#include <vector>

using namespace std::placeholders;

namespace yux {
namespace base {

class Peer
{
    public :
        Peer() : addr_(NULL) {}
        Peer(const char *host, uint16_t port) : host_(host), port_(port), addr_(NULL) {}
        ~Peer() { if (addr_) freeaddrinfo(addr_); }
        addrinfo* addr();
        std::string  host_;
        uint16_t     port_;
        addrinfo*    addr_;
};

class SocketBase
{
    public:
        enum SocketEvent
        {
            FIRST = 0,
            READ = FIRST,
            WRITE,
            CLOSE,
            LAST
        };

        // For call back. return 0 if OK. return -1 if error.
        typedef std::function<int (const char*, size_t, SocketBase*)> CallBack;
        virtual void regCallBack(SocketEvent event, CallBack cb) { callBackList_[event].push_back(cb); }
        virtual void regReadCallBack(CallBack cb) { regCallBack(SocketEvent::READ, cb); }

        SocketBase();
        SocketBase(int fd, const Peer& remote): fd_(fd), remote_(remote) {}
        virtual ~SocketBase();

        virtual void init() {}
        int fd() { return fd_; }
        const Peer& getLocal() { return local_; }
        const Peer& getRemote() { return remote_; }
        void setRemote(const char *host, uint16_t port) { remote_ = Peer(host, port); }
        void setNonBlocking();

        virtual int read() { return 0; }
        virtual int write(uint8_t* buf, size_t size) { return ::write(fd_, buf, size); }
        virtual int sendStr(const char* str) { return ::write(fd_, str, strlen(str)); }
        virtual int sendStr(const std::string& str) { return ::write(fd_, str.c_str(), str.size()); }
        virtual int send(uint8_t* buf, size_t size) { return ::write(fd_, buf, size); }
        virtual int recv(uint8_t* buf, size_t size) { return ::read(fd_, buf, size); }
        virtual void close();

    protected:
        #define BUF_SIZE 4096
        int fd_;
        char rdBuf_[BUF_SIZE];
        Peer remote_;
        Peer local_;
        std::vector<CallBack> callBackList_[SocketEvent::LAST];
};

// Client socket
class Socket : public SocketBase
{
    public:
        Socket() {}
        Socket(int fd, const Peer& remote) : SocketBase(fd, remote) {}
        Socket(const char *host, uint16_t port) { remote_ = Peer(host, port); }
        int connect();
        int connect(const char *host, uint16_t port) { remote_ = Peer(host, port); return connect(); }
        int read();
        int write(uint8_t* buf, size_t size);
};

// Server socket
class ServerSocket : public SocketBase
{
    public:

        ServerSocket() {}
        ServerSocket(const char* host, uint16_t port) { bind(host, port); listen(); }
        ServerSocket(const char* host, uint16_t port, SocketBase::CallBack cbRead) { bind(host, port); listen(); regCallBack(SocketEvent::READ, cbRead); }
        int bind(const char* host, uint16_t port);
        int listen() { return ::listen(fd_, 0); }
        SocketBase* accept();
};

}} //namespace

#endif

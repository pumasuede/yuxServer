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

using namespace std::placeholders;

namespace yux {
namespace base {

class Peer
{
    public :
        Peer() : addr_(NULL) {}
        Peer(const char *host, uint16_t port) : host_(host), port_(port), addr_(NULL) {}
        ~Peer() { if(addr_) freeaddrinfo(addr_); }
        addrinfo* addr();
        std::string  host_;
        uint16_t     port_;
        addrinfo*    addr_;
};

class SocketBase
{
    public:
        // For call back. return 0 if OK. return -1 if error.
        typedef std::function<int (const char*, size_t, SocketBase*)> CallBack;
        virtual void setCbRead(CallBack cb) {}
        virtual void setCbWrite(CallBack cb) { }

        SocketBase();
        SocketBase(int fd, const Peer& peer): fd_(fd), peer_(peer) {}
        virtual ~SocketBase() { close(); }

        virtual void init() {}
        int fd() { return fd_; }
        const Peer& getPeer() { return peer_; }
        void setPeer(const char *host, uint16_t port) { peer_ = Peer(host, port); }
        void setNonBlocking();

        virtual int read() {return 0;}
        virtual int write() = 0;
        virtual int sendStr(const char* str) { return ::write(fd_, str, strlen(str)); }
        virtual int sendStr(const std::string& str) { return ::write(fd_, str.c_str(), str.size()); }
        virtual int send(uint8_t* buf, size_t size) { return ::write(fd_, buf, size); }
        virtual int recv(uint8_t* buf, size_t size) { return ::read(fd_, buf, size); }
        void close();

    protected:
        #define BUF_SIZE 4096
        int fd_;
        char rdBuf_[BUF_SIZE];
        Peer peer_;
};

class Socket : public SocketBase
{
    public:
        void setCbRead(SocketBase::CallBack cb) { cbRead_ = cb; }
        void setCbWrite(SocketBase::CallBack cb) { cbWrite_ = cb; }

        Socket() {}
        Socket(int fd, const Peer& peer) : SocketBase(fd, peer) {}
        Socket(const char *host, uint16_t port) { peer_ = Peer(host, port); }
        int connect();
        int connect(const char *host, uint16_t port) { peer_ = Peer(host, port); return connect(); }
        int read();
        int write();
    private:
        CallBack cbRead_;
        CallBack cbWrite_;
};

class ServerSocket : public SocketBase
{
    public:
        static int readCallBack(char* buf, size_t size, SocketBase *sock);
        void setCbRead(SocketBase::CallBack cb) { cbRead_ = cb; }
        void setCbWrite(SocketBase::CallBack cb) { cbWrite_ = cb; }

        ServerSocket() {}
        ServerSocket(const char* host, uint16_t port) { bind(host, port); listen(); }
        ServerSocket(const char* host, uint16_t port, SocketBase::CallBack cbRead) : cbRead_(cbRead) { bind(host, port); listen(); }
        int bind(const char* host, uint16_t port);
        int listen(){ return ::listen(fd_, 0); }
        SocketBase* accept();
        int write();
    private:
        Peer self_;
        SocketBase::CallBack cbRead_;
        SocketBase::CallBack cbWrite_;
};

}} //namespace

#endif

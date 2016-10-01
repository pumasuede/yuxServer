#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>

#include <string>

#ifndef __YUX_SOCKET_H__
#define __YUX_SOCKET_H__

namespace yux
{
namespace base
{

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
        typedef int (*CbFun)(char*, size_t, SocketBase*, void*);
        virtual void setCbRead(CbFun pCb, void* pArgs) { }
        virtual void setCbWrite(CbFun pCb, void* pArgs) { }

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
        virtual int send(uint8_t* buf, size_t size) { return ::write(fd_, buf, size); }
        virtual int recv(uint8_t* buf, size_t size) { return ::read(fd_, buf, size); }
        void close();

    protected:
        int fd_;
        Peer peer_;
};

class Socket : public SocketBase
{
    public:
        // call backs must follow below definition.
        typedef struct {
            CbFun func;
            void* pArgs;
        } CallBack;

        void setCbRead(CbFun pCb, void* pArgs) { cbRead_.func = pCb; cbRead_.pArgs = pArgs; }
        void setCbWrite(CbFun pCb, void* pArgs) { cbWrite_.func = pCb; cbWrite_.pArgs = pArgs; }

        Socket() {}
        Socket(int fd, const Peer& peer) : SocketBase(fd, peer) {}
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
        ServerSocket() {}
        ServerSocket(const char* host, uint16_t port) { bind(host, port); listen(); }
        int bind(const char* host, uint16_t port);
        int listen(){ return ::listen(fd_, 0); }
        SocketBase* accept();
        int write();
    private:
        Peer self_;
};

}} //namespace

#endif

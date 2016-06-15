#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
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
        SocketBase();
        SocketBase(int fd, const Peer& peer): fd_(fd), peer_(peer) {}
        virtual ~SocketBase() { close(); }
        virtual void init() {}
        const Peer& getPeer() { return peer_; }
        void setPeer(const char *host, uint16_t port) { peer_ = Peer(host, port); }
        void setNonBlocking();
        void close();
        virtual int read() {} ;
        virtual int write() = 0;
        virtual int send(uint8_t* buf, size_t size) { return ::write(fd_, buf, size); }
        virtual int recv(uint8_t* buf, size_t size) { return ::read(fd_, buf, size); }
        int fd() { return fd_; }
    protected:
        int fd_;
        Peer peer_;
};

class Socket : public SocketBase
{
    public:
        Socket() {}
        Socket(int fd, const Peer& peer) : SocketBase(fd, peer) {}
        int connect();
        int connect(const char *host, uint16_t port) { peer_ = Peer(host, port); connect(); }
        int read();
        int write();
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

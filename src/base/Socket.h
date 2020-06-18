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
#include <mutex>

#include "base/Observable.h"

using namespace std::placeholders;

namespace yux {
namespace base {

class Peer
{
    public:
        Peer() : addr_(nullptr) { }
        Peer(const char *host, uint16_t port) : host_(host), port_(port), addr_(nullptr) {}
        ~Peer() { if (addr_) freeaddrinfo(addr_); addr_ = nullptr; }
        addrinfo* addr();
        std::string  host_;
        uint16_t     port_;
        addrinfo     *addr_;
};

class SocketBase;

class ISocketObserver
{
    public:
        virtual void onReadEvent(SocketBase* sock, const char* buff, size_t size) = 0;
        virtual void onOpenEvent(SocketBase* sock) = 0;
        virtual void onAcceptEvent(SocketBase* sock) = 0;
        virtual void onCloseEvent(SocketBase* sock) = 0;
        virtual ~ISocketObserver() {}
};

// Default implementation for ISocketObserver
// So derived class from SocketObserver doesn't need to implement call backs for all the events.
class SocketObserver : public ISocketObserver
{
    public:
        virtual void onReadEvent(SocketBase* sock, const char* buff, size_t size) {}
        virtual void onOpenEvent(SocketBase* sock) {}
        virtual void onAcceptEvent(SocketBase* sock) {}
        virtual void onCloseEvent(SocketBase* sock) {}
};

class SocketBase : public Observable<ISocketObserver>
{
    public:
        enum Status
        {
            CLOSED,
            OPEN,
            CONNECTED,
            LISTEN
        };

        SocketBase();
        // Create a socket object based on a opened socket(already has fd), which can be used by server.
        SocketBase(int fd, const Peer& remote): fd_(fd), remote_(remote), status_(OPEN) { }
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
        #define BUF_SIZE 8192
        int fd_;
        char rdBuf_[BUF_SIZE];
        Status status_;
        Peer remote_;
        Peer local_;
    private:
        void notifyCloseEvents(SocketBase* sock) { for (auto ob : observers_) { ob->onCloseEvent(sock); } }
};

// Client socket
class Socket : public SocketBase
{
    public:
        Socket(int fd, const Peer& remote) : SocketBase(fd, remote) {}
        Socket(ISocketObserver* pObserver = nullptr) { if (pObserver) { addObserver(pObserver); } }
        Socket(const char *host, uint16_t port, ISocketObserver* pObserver = nullptr) { remote_ = Peer(host, port); if (pObserver) { addObserver(pObserver); } }
        int connect();
        int connect(const char *host, uint16_t port) { remote_ = Peer(host, port); return connect(); }
        int read();
        int write(uint8_t* buf, size_t size);
    private:
        void notifyOpenEvents(SocketBase* sock) { for (auto ob : observers_) { ob->onOpenEvent(sock); } }
        void notifyReadEvents(SocketBase* sock, char* buf, size_t size) { for (auto ob : observers_) { ob->onReadEvent(sock, buf, size); } }
};

// Server socket
class ServerSocket : public SocketBase
{
    public:
        ServerSocket(ISocketObserver* pObserver = nullptr) { if (pObserver) { addObserver(pObserver);} }
        ServerSocket(const char* host, uint16_t port, ISocketObserver* pObserver = nullptr);
        int bind(const char* host, uint16_t port);
        int listen();
        SocketBase* accept();
    private:
        void notifyAcceptEvents(SocketBase* sock) { for (auto ob : observers_) { ob->onAcceptEvent(sock); } }
};

}} //namespace

#endif

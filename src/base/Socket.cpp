#include <strings.h>
#include <iostream>
#include <algorithm>

#include "Socket.h"
#include "Log.h"

using namespace std;

namespace yux {
namespace base {

addrinfo* Peer::addr()
{
    if (!addr_)
    {
       char port[8] = {0};
       sprintf(port, "%d", port_);
       getaddrinfo(host_.c_str(), port, NULL, &addr_);
    }

    return addr_;
}

SocketBase::SocketBase() : status_(CLOSED)
{
    fd_ = socket(PF_INET, SOCK_STREAM, 0);
    status_ = OPEN;
}

SocketBase::~SocketBase()
{
    log_debug("Socket %p is being deleted, fd_:%d, status:%d", this, fd_, status_);
    if (status_ == CLOSED)
        return;
    close();
}

void SocketBase::close()
{
    notifyCloseEvents(this);
    ::close(fd_);
    fd_ = -1;
    status_ = CLOSED;
}

void SocketBase::setNonBlocking()
{
    int opt;

    opt = fcntl(fd_, F_GETFL);
    if (opt < 0) {
        printf("fcntl(F_GETFL) fail.");
        exit(-1);
    }
    opt |= O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, opt) < 0) {
        printf("fcntl(F_SETFL) fail.");
        exit(-1);
    }
}

int Socket::connect()
{
    int rc = ::connect(fd_, remote_.addr()->ai_addr, remote_.addr()->ai_addrlen);
    if (rc != -1)
    {
        status_ = CONNECTED;
    }

    notifyOpenEvents(this);
    return rc;
}

int Socket::read()
{
    // data
    int readBytes = 0, offset = 0;
    bool closed = false;

    bzero(rdBuf_, sizeof(rdBuf_));

    log_trace("About to read socket...");
    int tmpSize = min(BUF_SIZE, 4096);

    while (offset <= BUF_SIZE - tmpSize)
    {
        readBytes = ::read(fd_, &rdBuf_[offset], tmpSize);
        if (readBytes > 0)
        {
            log_trace("read %d bytes", readBytes);
            offset += readBytes;
            continue;
        }
        else if (readBytes == 0)
        {
            log_debug("The socket is closed by remote peer\n");
            closed = true;
            break;
        }

        // readBytes < 0 : Handle read error

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Reading buffer is complete, no available data
            log_trace("errno == EAGAIN when reading socket buffer - all available data is read complete");
            break;
        }
        else if (errno == EINTR)
        {
            log_trace("errno == EINTR when reading socket buffer");
            continue;
        }
        else
        {
            // True Error
            log_error("Error when reading socket buffer! errno=%d error str:%s ", errno, strerror(errno));
            return -1;
        }
    }

    // read completed
    // std::cout<<"Buf received: "<<std::to_string(offset)<<" bytes\n";
    log_trace("Socket::read - received %d bytes", offset);

    if (closed)
        return 0;

    // Do callback for READ event
    notifyReadEvents(this, rdBuf_, offset);

    return 1;
}

int Socket::write(uint8_t* buf, size_t size)
{
    int offset = 0;
    int writeBytes = 0;

    while (offset < size)
    {
        int tmpSize = min(int(size - offset), 4096);
        writeBytes = ::write(fd_, &buf[offset], tmpSize);
        if (writeBytes > 0)
        {
            log_trace("write %d bytes", writeBytes);
            offset += writeBytes;
            continue;
        }
        // writeBytes < 0 : Handle read error
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Wrting buffer is complete.
            log_trace("errno == EAGAIN when writing socket buffer ");
            continue;
        }
        else if (errno == EINTR)
        {
            log_trace("errno == EINTR when writing socket buffer");
            continue;
        }
        else
        {
            log_error("Error when writing socket buffer! errno=%d error str:%s", errno, strerror(errno));
            return -1;
        }
    }

    log_trace("Socket::write - sent %d bytes to client", offset);
    return 0;
}

ServerSocket::ServerSocket(const char* host, uint16_t port, ISocketObserver* pObserver)
{
    if (pObserver)
    {
        addObserver(pObserver);
    }

    bind(host, port);

    listen();
}

int ServerSocket::bind(const char* host, uint16_t port)
{
   local_ = Peer(host, port);
   int rc = ::bind(fd_, local_.addr()->ai_addr, local_.addr()->ai_addrlen);

   if (rc == -1)
   {
       std::cout<<"Socket bind failed on port "<<port<<" : "<<strerror(errno) <<"\n";
       exit(-1);
   }

   return rc;
}

int ServerSocket::listen()
{
    int rc = ::listen(fd_, 0);
    if (rc == -1)
    {
        std::cout<<"Socket listen failed on port "<<local_.port_<<" : "<<strerror(errno) <<"\n";
        exit(-1);
    }

    status_ = LISTEN;
    return rc;
}

SocketBase* ServerSocket::accept()
{
    sockaddr_in  clientAddr;
    socklen_t    addrLen = sizeof(clientAddr);
    int newFd = ::accept(fd_, (sockaddr*) &clientAddr, &addrLen);
    log_debug("New client from [%s] on Fd %d", inet_ntoa(clientAddr.sin_addr), newFd);

    Peer remote(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    SocketBase *pSock = new Socket(newFd, remote);
    pSock->setNonBlocking();

    // Do callback for ACCEPT event
    notifyAcceptEvents(pSock);

    // Observer of listen socket should also observe spawned data sockets
    for (auto ob : observers_)
    {
        pSock->addObserver(ob);
    }

    return pSock;
}

}}

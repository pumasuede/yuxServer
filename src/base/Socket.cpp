#include <strings.h>
#include <iostream>
#include <algorithm>

#include "Socket.h"
#include "Log.h"

using namespace std;

namespace yux
{
namespace base
{

addrinfo* Peer::addr()
{
    if (!addr_)
    {
       char port[8] = {0};
       sprintf(port, "%d", port_);
       addr_ = new addrinfo();
       getaddrinfo(host_.c_str(), port, NULL, &addr_);
    }

    return addr_;
}

SocketBase::SocketBase()
{
    fd_ = socket(PF_INET, SOCK_STREAM, 0);
}

void SocketBase::close()
{
    ::close(fd_);
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
    return ::connect(fd_, peer_.addr()->ai_addr, peer_.addr()->ai_addrlen);
}

int Socket::read()
{
    // data
    int readBytes = 0, offset = 0;
    bool closed = false;

    bzero(rdBuf_, sizeof(rdBuf_));

    log_debug("About to read socket...");
    int tmpSize = min(BUF_SIZE, 4096);

    while (offset <= BUF_SIZE - tmpSize)
    {
        readBytes = ::read(fd_, &rdBuf_[offset], tmpSize);
        if (readBytes > 0)
        {
            log_debug("read %d bytes", readBytes);
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
            log_debug("errno == EAGAIN when reading socket buffer - all available data is read complete");
            break;
        }
        else if (errno == EINTR)
        {
            log_debug("errno == EINTR when reading socket buffer");
            continue;
        }
        else
        {
            // True Error
            cout<<"Error when reading socket buffer! errno="<<errno<<endl;
            return -1;
        }
    }

    // read completed , do callback
    std::cout<<"Buf received: "<<std::to_string(offset)<<" bytes\n";
    log_debug("Socket::read - received %d bytes", offset);

    int ret = cbRead_(rdBuf_, offset, this);
    if (closed)
        return 0;

    return ret == -1 ? -1 : 1;
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
            log_debug("write %d bytes", writeBytes);
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
            cout<<"Error when writing socket buffer! errno="<<errno<<endl;
            return -1;
        }
    }

    log_debug("Socket::write - sent %d bytes to client", offset);
}

int ServerSocket::bind(const char* host, uint16_t port)
{
   peer_ = Peer(host, port);
   int rc = ::bind(fd_, peer_.addr()->ai_addr, peer_.addr()->ai_addrlen);

   if (rc == -1)
   {
       std::cout<<"Socket bind failed on port "<<port<<" : "<<strerror(errno) <<"\n";
       exit(-1);
   }

   return rc;
}

int ServerSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string recvBuf(buf, size);
    cout<<"read "<<size<<" bytes:"<<buf<<endl;
    log_info("read %d bytes - [%s]", size, buf);
    return 1;
}

SocketBase* ServerSocket::accept()
{
    sockaddr_in  clientAddr;
    socklen_t    addrLen = sizeof(clientAddr);
    int newFd = ::accept(fd_, (sockaddr*) &clientAddr, &addrLen);
    cout<<"new client from ["<<inet_ntoa(clientAddr.sin_addr)<<":"<<ntohs(clientAddr.sin_port)<<"]...\n";
    Peer peer(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    SocketBase *sock = new Socket(newFd, peer);
    sock->setNonBlocking();
    sock->setCbRead(cbRead_);

    return sock;
}

ServerSocket* ServerSocket::create(const std::string& host, uint16_t port)
{
    ServerSocket* pSock = new ServerSocket(host.c_str(), port);
    pSock->setCbRead(std::bind(&ServerSocket::readCallBack, pSock, _1, _2, _3));
    return pSock;
}

}}

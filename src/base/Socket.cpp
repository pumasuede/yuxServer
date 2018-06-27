#include <strings.h>
#include <iostream>
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
    int readBytes = 0, pos = 0;
    bool closed = false;

    bzero(rdBuf_, sizeof(rdBuf_));

    cout<<"Socket::read - About to read socket...\n";
    int tmpSize = BUF_SIZE < 1024 ? BUF_SIZE : 1024;

    while (pos <= BUF_SIZE - tmpSize)
    {
        readBytes = ::read(fd_, &rdBuf_[pos], tmpSize);
        if (readBytes > 0)
        {
            cout<<"read "<<readBytes<<"bytes\n";
            pos += readBytes;
            continue;
        }
        else if (readBytes == 0)
        {
            cout<<"The socket is closed by remote peer\n";
            closed = true;
            break;
        }

        // readRet < 0 : Handle read error

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Reading buffer is complete, no available data
            log_debug("errno == EAGAIN when reading socket buffer - all available data is read");
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
    log_debug("Buf received:%d bytes", pos);
    cout<<"Socket::read - buf received: "<<pos<<" bytes\n";

    int ret = cbRead_(rdBuf_, pos, this);
    if (closed)
        return 0;

    return ret == -1 ? -1 : 1;
}

int Socket::write()
{
    return fd_;
}

int ServerSocket::bind(const char* host, uint16_t port)
{
   peer_ = Peer(host, port);
   int rc = ::bind(fd_, peer_.addr()->ai_addr, peer_.addr()->ai_addrlen);

   if (rc == -1)
   {
       std::cout<<"Socket bind failed on port "<<port<<" : "<<strerror(errno) <<"\n";
       throw strerror(errno);
   }

   return rc;
}

int ServerSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string recvBuf(buf, size);
    cout<<"read "<<size<<" bytes:"<<buf<<endl;
    log_debug("read %d bytes - [%s]", size, buf);
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

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
    int readRet = 0, pos = 0;

    bzero(rdBuf_, sizeof(rdBuf_));

    cout<<"About to read socket...\n";
    int tmpSize = BUF_SIZE < 1024 ? BUF_SIZE : 1024;

    while ( pos <= BUF_SIZE - tmpSize )
    {
        readRet = ::read(fd_, &rdBuf_[pos], tmpSize);
        if (readRet > 0)
        {
            pos += readRet;
            continue;
        }
        else if (readRet == 0)
        {
            cout<<"The socket is closed by remote peer\n";
            return -1;
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
    cout<<"->Buf received: "<<pos<<" bytes\n";
    return cbRead_(rdBuf_, pos, this);
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

int ServerSocket::readCallBack(char* buf, size_t size, SocketBase *sock)
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

int ServerSocket::write()
{
    return fd_;
}

}}

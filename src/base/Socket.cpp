#include <strings.h>
#include <iostream>
#include "Socket.h"

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

    char buf[10000];
    bzero(buf, sizeof(buf));

    while ( (readRet = ::read(fd_, &buf[pos], 1024)) > 0 )
    {
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


        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            //reading buffer is complete, no available data
            cout<<"buf received: "<<pos<<"bytes\n";
            break;
        }
        if (errno == EINTR)
            continue;
    }

    return cbRead_.func(buf, pos, this, cbRead_.pArgs);

    return 0;
}

int Socket::write()
{
    return fd_;
}

int ServerSocket::bind(const char* host, uint16_t port)
{
   //cout<<"bind IP:"<<host<<" port:"<<port<<"\n";
   peer_ = Peer(host, port);

   return ::bind(fd_, peer_.addr()->ai_addr, peer_.addr()->ai_addrlen);
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

    return sock;
}

int ServerSocket::write()
{
    return fd_;
}

}}

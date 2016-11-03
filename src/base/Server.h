#ifndef __YUX_SERVER_H__
#define __YUX_SERVER_H__

#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "Socket.h"

namespace yux{
namespace base{
class Fdes;

class Server
{
    public:
        static int readCallBack(char* buf, size_t size, SocketBase *sock, void *pArgs);

        Server(std::string host, uint16_t port, SocketBase::CbFun cbRead=&readCallBack);
        virtual ~Server();
        void loop();
        void loopOnce();
        void stop() { stop_ = true; }
        int getListenFd();
        void closeSocket(SocketBase* sock);

    private:
        ServerSocket *servSock_;
        std::vector<SocketBase*> fdToSkt_;
        Fdes* fdes_;
        bool stop_;
        SocketBase::CbFun cbRead_;
};

}} //namespace

#endif

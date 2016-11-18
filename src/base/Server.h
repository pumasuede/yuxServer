#ifndef __YUX_SERVER_H__
#define __YUX_SERVER_H__

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <set>

#include "Socket.h"

namespace yux{
namespace base{
class Fdes;

class Server
{
    public:
        static Server& getInstance();
        void init(std::string host, uint16_t port, SocketBase::CallBack cbRead);
        void init();
        void addServerSocket(SocketBase* pServerSocket);
        ~Server();
        void loop();
        void loopOnce();
        void stop() { stop_ = true; }
        void closeSocket(SocketBase* sock);

    private:
        Server() {};
        std::set<SocketBase*> servSockList_;
        std::vector<SocketBase*> fdToSkt_;
        Fdes* fdes_;
        bool stop_;
};

}} //namespace

#endif

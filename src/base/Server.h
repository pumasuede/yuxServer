#ifndef __YUX_SERVER_H__
#define __YUX_SERVER_H__

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <list>
#include <set>
#include <mutex>

#include "Utils.h"
#include "Socket.h"

#define HTTP_SERVER_VERSION "1.0.0"

namespace yux{
namespace base{
class Fdes;

class Server
{
    public:
        static Server& getInstance();
        ~Server();
        void init(std::string host, uint16_t port, SocketBase::CallBack cbRead, Timer* timer = NULL);
        void init();
        void loop();
        void loopOnce();
        void stop() { stop_ = true; }
        void addTimer(Timer* timer);
        void addTimer(int intval, Timer::TimerCallBack timerCb);
        void addServerSocket(SocketBase* pServerSocket);
        void regSocket(SocketBase* pSocket) { fdToSkt_[pSocket->fd()] = pSocket; }
        void closeSocket(SocketBase* sock) { closeSocket(sock->fd()); }
        void closeSocket(int fd);
        SocketBase* getSocketByFd(int fd) { return fdToSkt_[fd]; }

    private:
        Server() {};
        Timer* getMinTimer(int current);
        std::set<SocketBase*> servSockList_;
        std::vector<SocketBase*> fdToSkt_;
        std::list<Timer*> timers_;
        Fdes* fdes_;
        bool stop_;
        std::mutex mutex_;
};

}} //namespace

#endif

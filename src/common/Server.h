#ifndef __YUX_SERVER_H__
#define __YUX_SERVER_H__

#include <unistd.h>
#include <stdint.h>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <mutex>

#include "base/Timer.h"
#include "base/Socket.h"
#include "base/Fde.h"
#include "Utils.h"

#define HTTP_SERVER_VERSION "1.0.0"

namespace yux {
namespace common {

class Server
{
    public:
        typedef yux::base::SocketBase SocketBase;
        typedef yux::base::Timer Timer;

        static Server& getInstance();
        ~Server();
        void init(std::string host, uint16_t port, const SocketBase::CallBack& cbRead, Timer* timer = NULL);
        void init();
        void loop();
        void loopOnce();
        void stop() { stop_ = true; }

        // Add a timer to Server. it will be deleted when the server exits.
        void addTimer(Timer* timer);
        void addTimer(int intval, const Timer::TimerCallBack& timerCb);

        // Add a server socket to Server. The socket will be managed by Server and delelted when the server exits.
        void addServerSocket(SocketBase* pServerSocket);

        // Register a socket to Server. The socket will be managed by Server and delelted when it's closed.
        void regSocket(SocketBase* pSocket);

        void closeSocket(SocketBase* pSocket);
        std::shared_ptr<SocketBase> getSocketByFd(int fd) { std::lock_guard<std::mutex> lock(mutex_); return fdToSkt_[fd]; }

    private:
        Server() {};
        yux::base::Timer* getMinTimer(int current);
        std::set<SocketBase*> servSockList_;
        std::vector<std::shared_ptr<SocketBase>> fdToSkt_;
        std::list<Timer*> timers_;
        yux::base::Fdes* fdes_;
        bool stop_;
        std::mutex mutex_;
};

}} //namespace

#endif

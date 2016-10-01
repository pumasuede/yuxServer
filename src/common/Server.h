#include <unistd.h>
#include <stdint.h>
#include <vector>

namespace yux{
namespace base{
class SocketBase;
class Socket;
class ServerSocket;
class Fdes;
}}

namespace yux{
namespace common{

class Server
{
    public:
        Server(std::string host, uint16_t port);
        static int readCallBack(char* buf, size_t size, yux::base::SocketBase *sock, void *pArgs);
        ~Server();
        void loop() { while(1) {loopOnce();} }
        int getListenFd();

    private:
        void loopOnce();
        yux::base::ServerSocket *servSock_;
        std::vector<yux::base::Socket*>  socks_;
        std::vector<yux::base::SocketBase*> fdToSkt_;
        yux::base::Fdes* fdes_;
};

}}

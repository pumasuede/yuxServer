#include <unistd.h>
#include <stdint.h>
#include <vector>

#include "base/Socket.h"

namespace yux{
namespace common{

class YuxServerSocket : public yux::base::ServerSocket
{
public:
    int readCallBack(const char* buf, size_t size, yux::base::SocketBase *sock);
    static YuxServerSocket* create(const char* host, uint16_t port);
private:
    YuxServerSocket(const char* host, uint16_t port) : ServerSocket(host, port) {}
};

}}

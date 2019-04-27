#include <errno.h>

#include "base/Log.h"
#include "base/Timer.h"
#include "common/Config.h"
#include "common/Server.h"
#include "http/HttpClientSocket.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

namespace yux {
namespace http{

//Define timer call backs. The first arg is func name, the second is time interval measured by second.
DECL_TIMER(Timer1Callback, 120*1000)
DECL_TIMER(Timer2Callback, 10*1000)

int httpClient_readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    log_trace("readCallBack %d bytes", size);
    char pageFile[] = "baidu.html";

    int fd = open(pageFile, O_RDWR|O_APPEND);
    if (fd == -1 && errno == ENOENT)
    {
        fd = open(pageFile, O_RDWR|O_CREAT);
    }
    write(fd, buf, size);
    close(fd);
    return 0;
}

int Timer1Callback(void* pServer)
{
    const char *hostname = "www.baidu.com";
    log_debug("*** Timer1Callback called. Try to request %s", hostname);

    HttpClientSocket *httpClient = new HttpClientSocket;
    Server::getInstance().regSocket(httpClient);
    httpClient->request(hostname, std::bind(&httpClient_readCallBack, _1, _2, _3));
    return 0;
}


// Timer2
int Timer2Callback(void*)
{
    log_debug("*** Timer2Callback called ");
    return 0;
}

}} // name space

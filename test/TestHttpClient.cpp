#include <iostream>
#include "base/Socket.h"
#include "base/Fde.h"
#include "base/Log.h"
#include "http/HttpClientSocket.h"

using namespace std;
using namespace yux::base;
using namespace yux::http;

int readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    string data(buf, size);
    //std::cout<<"received "<<size<<" bytes in call back\n";
    std::cout<<data<<"\n";
    return 0;
}

void timerCallBack(void*)
{
    cout<<"timerCallBack";
}

void testHttpClient(const char *url)
{
    HttpClientSocket httpClient;
    httpClient.request(url, std::bind(&readCallBack, _1, _2, _3));
}

int main(int argc, char** argv)
{
    log_open("test.log");
    const char *url = argc > 1 ? argv[1] : "baidu.com";
    testHttpClient(url);

    return 0;
}

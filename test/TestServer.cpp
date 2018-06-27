#include <iostream>
#include <functional>

#include "base/Server.h"

using namespace std;
using namespace yux::base;

int readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    // custom call back handling
    string recvBuf(buf, size);
    cout<<"read callback - "<<size<<" bytes:"<<buf<<endl;
    FILE * fp;
    char buffer[80];
    fp=popen(recvBuf.c_str(), "r");
    while (fgets(buffer, sizeof(buffer),fp))
    {
        cout<<buffer;
        sock->sendStr(buffer);
    }
    sock->sendStr(">");
    pclose(fp);
    return 1;
}

int main(int argc, char* argv[])
{
    int port = 9999;
    if (argc > 1)
        port = atoi(argv[1]);

    Server& mainServer = Server::getInstance();
    mainServer.init("0.0.0.0", port, std::bind(readCallBack, _1, _2, _3));
    mainServer.loop();
    return 0;
}

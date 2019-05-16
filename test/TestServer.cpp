#include <strings.h>
#include <iostream>
#include <functional>

#include "base/Log.h"
#include "common/Server.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

class MySocketObserver : public SocketObserver
{
    public:
        void onCloseEvent(SocketBase *sock) { cout<<"socket closed... fd:"<<sock->fd()<<" \n"; }
        void onAcceptEvent(SocketBase *sock);
        void onReadEvent(SocketBase *sock, const char* buf, size_t size);
    private:
        void sendWorkPath(SocketBase *sock);
        char workPath_[1024];
};

void MySocketObserver::sendWorkPath(SocketBase *sock)
{
    getcwd(workPath_, sizeof(workPath_));
    sock->sendStr(workPath_);
    sock->sendStr(">");
}

void MySocketObserver::onAcceptEvent(SocketBase *sock)
{
    sendWorkPath(sock);
}

void MySocketObserver::onReadEvent(SocketBase *sock, const char* buf, size_t size)
{
    // Custom call back handling
    string recvBuf(buf, size);
    cout<<"read callback - "<<size<<" bytes:"<<buf<<endl;
    recvBuf[size-1] = 0;
    FILE * fp;
    char buffer[1024];

    // cd should be executed in current process
    if (strncasecmp(recvBuf.c_str(), "cd ", 3) == 0)
    {
        const char* dir = recvBuf.substr(3).c_str();
        chdir(dir);
        sendWorkPath(sock);
        return;
    }

    fp = popen(recvBuf.c_str(), "r");
    while (fgets(buffer, sizeof(buffer),fp))
    {
        cout<<buffer;
        sock->sendStr(buffer);
    }

    sendWorkPath(sock);
    pclose(fp);
}

int main(int argc, char* argv[])
{
    log_open("test.log");
    int port = 9999;
    if (argc > 1)
        port = atoi(argv[1]);

    Server* mainServer = Server::getInstance();
    mainServer->init("0.0.0.0", port, new MySocketObserver);
    mainServer->loop();

    return 0;
}

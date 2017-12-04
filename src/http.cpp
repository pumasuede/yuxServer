#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include "base/Log.h"
#include "base/Server.h"
#include "common/YuxServerSocket.h"
#include "common/HttpServerSocket.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

#define DEFAUL_PORT  8080
string server_ip = "0.0.0.0";
uint16_t server_port = DEFAUL_PORT;

void print_help(const char* name)
{
    cout<<"Usage: "<<name<<" -ap \n";
    cout<<"-a --ip  Server IP\n";
    cout<<"-p --port  Server port\n";
}

int parse_arg(int argc, char* argv[])
{
    struct option long_opts[] =
    {
        {"ip", 1, 0, 'a'},
        {"port", 1, 0, 'p'},
        {0, 0, 0, 0}
    };

    int c;

    while (1)
    {
        int idx = 0;
        c = getopt_long(argc, argv, "a:p:", long_opts, &idx);
        if (c<0)
            break;
        switch ( c )
        {
            case 'a':
                server_ip = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            default:
                print_help(argv[0]);
                exit(1);
                break;
        }
    }
    return c;
}

int Timer1Callback(void*)
{
    cout<<"*** Timer1Callback called \n ";
    log_debug("*** Timer1Callback called ");
}


int main(int argc, char* argv[])
{
    log_open("httpd.log");
    parse_arg(argc, argv);

    Server& mainServer = Server::getInstance();
    mainServer.init();

    Timer *timer = new Timer(5000, &Timer1Callback);
    mainServer.addTimer(timer);

    SocketBase* httpServerSock = HttpServerSocket::create(server_ip, server_port);
    mainServer.addServerSocket(httpServerSock);

    mainServer.loop();

    return 0;
}

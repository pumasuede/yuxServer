#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#ifdef __linux__
    #include <sys/epoll.h>
#endif
#include "common/Server.h"

using namespace std;
using namespace yux::common;

#define DEFAUL_PORT  8000
string server_ip = "0.0.0.0";
uint16_t server_port = DEFAUL_PORT;

void print_help(const char* name)
{
    cout<<"Usage: "<<name<<" -ap \n";
    cout<<"--ip  Server IP\n";
    cout<<"--port  Server port\n";
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
        c = getopt_long( argc, argv, "a:p:", long_opts, &idx);
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

int main(int argc, char* argv[])
{
    parse_arg(argc, argv);
    Server server(server_ip, server_port);
    server.loop();

    return 0;
}

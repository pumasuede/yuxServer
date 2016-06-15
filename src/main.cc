#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "base/Socket.h"

using namespace std;
using namespace yux::base;

#define DEFAUL_PORT  8000
string server_ip;
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

    while (1)
    {
        int idx = 0;
        int c = getopt_long( argc, argv, "a:p:", long_opts, &idx);
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
}

int main(int argc, char* argv[])
{
    parse_arg(argc, argv);
    //new EchoServer
    std::vector<SocketBase*> fdToSkt;
    // Don't keep track of the "max fd" we've seen, just get the max possible and use that
    rlimit r;
    if (-1 == getrlimit( RLIMIT_NOFILE, &r ))
        return -1;
    uint32_t fdMax = r.rlim_max;

    // Initialize all IOBaseBase * to NULL
    fdToSkt.resize(fdMax);
    for ( int n = 0; n < fdMax; ++n )
        fdToSkt[n] = NULL;

    ServerSocket servSock(server_ip.c_str(), server_port);
    cout<<"listen on IP:"<<servSock.getPeer().host_<<" and Port:"<<servSock.getPeer().port_<<"...\n";


    //server.listen

    uint16_t listenFd = servSock.fd();
    fdToSkt[listenFd] = &servSock;

    int epollFd_ = epoll_create(10);

    struct epoll_event ee_ = { 0 };
    ee_.events = EPOLLIN;
    ee_.data.fd  = listenFd;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, listenFd, &ee_);

    const int ee_size = 10;
    struct epoll_event *events = new struct epoll_event[ee_size];

    //server.loop

    while (1)
    {
        int n = epoll_wait(epollFd_, events, ee_size, -1);
        if (n < 0 && errno == EINTR)
            continue;

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            SocketBase *skt = fdToSkt[fd];
            SocketBase *newSkt = skt; // set the same value

            if (!skt)
                continue;

            if ( events[i].events & EPOLLOUT )
            {
                skt->write();
            }
            else
            {
                if (fd == servSock.fd())
                {
                    cout<<"ret newSkt: "<<newSkt<<" listenFd:"<<fd<<"...\n";
                    SocketBase *newSkt = dynamic_cast<ServerSocket*>(skt)->accept();
                    int newFd = newSkt->fd();
                    fdToSkt[newFd] = newSkt;

                    struct epoll_event ee_ = { 0 };
                    ee_.events = EPOLLIN;
                    ee_.data.fd  = newFd;
                    epoll_ctl(epollFd_, EPOLL_CTL_ADD, newFd, &ee_);

                }
                else
                {
                    cout<<"read on client socket "<<fd<<" \n";
                    int ret = skt->read();
                    if (ret < 0)
                    {
                        cout<<"abnormal in Socket read, will delete socket..\n";
                        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, NULL);
                        fdToSkt[fd] = NULL;
                        delete skt;
                    }

                }
            }
        }
    }

    return 0;
}

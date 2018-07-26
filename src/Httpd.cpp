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
#include "base/Config.h"
#include "base/Server.h"
#include "http/HttpServerSocket.h"

using namespace std;
using namespace yux::base;
using namespace yux::http;

#define DEFAULT_PORT  8080
string serverIP = "0.0.0.0";
uint16_t serverPort = DEFAULT_PORT;
string configFile = "config";

void print_help(const char* name)
{
    cout<<"Usage: "<<name<<" -ap \n";
    cout<<"-p --port  Server port\n";
    cout<<"-c --config  Config file path\n";
}

int parse_arg(int argc, char* argv[])
{
    struct option long_opts[] =
    {
        {"port", 1, 0, 'p'},
        {"config", 1, 0, 'c'},
        {0, 0, 0, 0}
    };

    int c;

    while (1)
    {
        int idx = 0;
        c = getopt_long(argc, argv, "p:c:", long_opts, &idx);
        if (c<0)
            break;
        switch ( c )
        {
            case 'p':
                serverPort = atoi(optarg);
                break;
            case 'c':
                configFile = optarg;
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

int Timer2Callback(void*)
{
    cout<<"*** Timer2Callback called \n ";
    log_debug("*** Timer2Callback called ");
}

class MainThread : public Thread
{
    public:
        MainThread() : Thread("main thread") {}
        void workBody();
};

void MainThread::workBody()
{
    Server& mainServer = Server::getInstance();
    mainServer.init();

    Timer *timer1 = new Timer(5000, &Timer1Callback);
    Timer *timer2 = new Timer(3000, &Timer2Callback);
    //mainServer.addTimer(timer1);
    //mainServer.addTimer(timer2);

    HttpServerSocket* httpServerSock = HttpServerSocket::create(serverIP, serverPort);
    mainServer.addServerSocket(httpServerSock);
    string docRoot = Config::getInstance()->get("document_root", ".");

    httpServerSock->setDocRoot(docRoot);

    const int httpWorkThreadNum = 5;
    for (int i = 0; i < httpWorkThreadNum; i++)
    {
        HttpServerThread *pThread = new HttpServerThread("Http wotk thread"+std::to_string(i), httpServerSock);
        pThread->start();
    }

    mainServer.loop();
}

int main(int argc, char* argv[])
{
    parse_arg(argc, argv);

    // Load config
    Config* pConfig = Config::getInstance();
    pConfig->loadConfigFile(configFile);
    string port = pConfig->get("port", std::to_string(DEFAULT_PORT));
    serverPort = stoi(port);
    string logFile = pConfig->get("log_file", "httpd.log");

    // Open log
    log_open(logFile.c_str());

    Thread *main = new MainThread();

    main->start();
    main->join();

    return 0;
}

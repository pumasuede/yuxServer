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
#include "base/Timer.h"
#include "common/Config.h"
#include "common/Server.h"

#include "http/HttpService.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;
using namespace yux::http;

#define DEFAULT_PORT  8080
string serverIP = "0.0.0.0";
uint16_t serverPort = DEFAULT_PORT;
string configFile = "config";

void printHelp(const char* name)
{
    cout<<"Usage: "<<name<<" -ap \n";
    cout<<"-p --port  Server port\n";
    cout<<"-c --config  Config file path\n";
}

int parseArg(int argc, char* argv[])
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

        if (c < 0)
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
                printHelp(argv[0]);
                exit(1);
                break;
        }
    }
    return c;
}

class MainThread : public Thread
{
    public:
        MainThread() : Thread("main thread") {}
        void workBody();
};

void MainThread::workBody()
{
    Server* mainServer = Server::getInstance();
    mainServer->init();

    for (auto& timer : getTimers())
    {
        mainServer->addTimer(&timer);
    }

    HttpService* httpService = HttpService::create(serverIP, serverPort);
    mainServer->addServerSocket(httpService->getServerSocket());

    ThreadManager::getInstance()->dump();

    mainServer->loop();
}

int main(int argc, char* argv[])
{
    parseArg(argc, argv);

    // Load config
    Config* pConfig = Config::getInstance();
    pConfig->loadConfigFile(configFile);
    string port = pConfig->get("port", std::to_string(DEFAULT_PORT));
    serverPort = stoi(port);
    string logFile = pConfig->get("log_file", "httpd.log");
    string logLevel = pConfig->get("log_level", "INFO");

    // Open log
    log_open(logFile.c_str(), Logger::getLevel(logLevel));

    log_info("config file :%s", configFile.c_str());

    Thread *main = new MainThread();

    main->start();

    return 0;
}

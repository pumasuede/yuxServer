#include "base/Log.h"
#include "base/Thread.h"

using namespace yux::base;

class MainThread : public Thread
{
    public:
        MainThread() : Thread("main thread") {}
        void workBody();
};

void MainThread::workBody()
{
    Thread *pThread = new Thread("work thread", this);
    pThread->start();
    pThread->detach();

    int i = 0;
    while (1)
    {
        log_info("main loop in main thread %d", i++);
        sleep(1);
    }
}

int main()
{
    log_open("log/my.log", Logger::LEVEL_MAX, 2*1024*1024);

    Thread *main = new MainThread();

    main->start();

    //log_debug("line %d\n", 1);
}

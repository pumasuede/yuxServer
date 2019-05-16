#include "base/Log.h"
#include "base/Thread.h"
#include "base/Singleton.h"

using namespace yux::base;

class MainThread : public Thread, public Singleton<MainThread>
{
    friend class Singleton<MainThread>;
    public:
        void workBody();
    private:
        MainThread() : Thread("main thread") {}
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

void testThread()
{
    MainThread::getInstance()->start();
}

int main()
{
    log_open("log/my.log", Logger::LEVEL_MAX, 2*1024*1024);
    testThread();
}

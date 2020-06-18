#include "base/Log.h"
#include "base/Thread.h"
#include "base/Scheduler.h"
#include "base/Singleton.h"

using namespace yux::base;

class MainThread : public Thread, public Singleton<MainThread>
{
    friend class Singleton<MainThread>;
    public:
        void workBody();
        void callBack(void *pArg);
    private:
        MainThread() : Thread("main thread") {}
};

void MainThread::callBack(void* pArg)
{
    log_info("callback in main thread ");
}

void MainThread::workBody()
{
    Scheduler *pScheduler = new Scheduler("WorkScheduler", 4);

    int i = 0;
    while (1)
    {
        log_info("main loop in main thread %d", i++);
        WorkItem workItem(std::bind(&MainThread::callBack, this, nullptr));
        pScheduler->enqueueWorkItem(workItem);
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

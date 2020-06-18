#include "Thread.h"
#include "Scheduler.h"

using namespace std;

namespace yux {
namespace base {

Thread::Thread(const std::string& name, Scheduler *pScheduler) :
    name_(name), pScheduler_(pScheduler), stop_(false), thr_(nullptr)
{
    id_ = Singleton<ThreadManager>::getInstance()->addThread(this);
    log_trace("Thread %s [%d] is created", name.c_str(), id_);
}

Thread::~Thread()
{
    log_trace("Removing thread %s [%d]...", name_.c_str(), id_);
    Singleton<ThreadManager>::getInstance()->dropThread(this);
    delete thr_;
}

void Thread::threadEntry()
{
    didStart();
    workBody();
}

void Thread::workBody()
{
    int cnt = 0;
    while (!stop_)
    {
        if (pScheduler_)
        {
            WorkItem workItem = pScheduler_->getWorkItem();
            workItem.run();
        }
        else
        {
            // A thread which does not belong to ang scheduler.
            // print the thread info as work
            log_debug("---workd body: %s Tid 0x%x wake times : %d",
                      name_.c_str(), tid_, cnt++);
            usleep(cnt/3*1000*100);
        }
    }
}

// Thread manager
ThreadManager::~ThreadManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& thr: threadTable_)
    {
        delete thr.second;
    }
}

void ThreadManager::dropThread(Thread *pThr)
{
    std::lock_guard<std::mutex> lock(mutex_);

    threadTable_.erase(pThr->getId());
}

size_t ThreadManager::addThread(Thread *pThr)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if ( nextId_ > 65535 )
        nextId_ = 0;

    threadTable_[nextId_] = pThr;
    return nextId_++;
}

void ThreadManager::dump()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& thr : threadTable_)
    {
        log_info("*** Thread (%s) id: %d Tid: %x ***", thr.second->getName().c_str(), thr.second->getId(), thr.second->getTid());
    }
}

}}

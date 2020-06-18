#include "Scheduler.h"
#include "Thread.h"
#include "Log.h"

using namespace std;
using namespace yux::base;

namespace yux {
namespace base {

Scheduler::Scheduler(const string& name, int numThread):
    name_(name)
{
    for (int i = 0; i < numThread; i++)
    {
        Thread *pThread = new Thread(name+".Worker"+std::to_string(i), this);
        pThread->start();
        pThread->detach();
        threads_.push_back(pThread);
    }
}

Scheduler::~Scheduler()
{
    for (auto& thread : threads_)
    {
        thread->stop();
    }
}

void Scheduler::enqueueWorkItem(const WorkItem& workItem)
{
    log_info("Adding a work item to scheduler");
    lock_guard<mutex> lock(mutex_);
    workItems_.push(workItem);
    cv_.notify_all();
}

WorkItem Scheduler::getWorkItem()
{
    log_info("Fetching a work item to process");
    unique_lock<mutex> lock(mutex_);
    cv_.wait(lock, [&]{ return !workItems_.empty(); });
    WorkItem workItem = workItems_.front();
    workItems_.pop();
    return workItem;
}

}}

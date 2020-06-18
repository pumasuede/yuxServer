#ifndef __YUX_SCHEDULER_H__
#define __YUX_SCHEDULER_H__

#include <string>
#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

#define DEFAULT_THREAD_NUM 5

namespace yux {
namespace base {

// A work item consists of a call back function and args

class Thread;

class WorkItem
{
    public:
        WorkItem(const std::function<void ()>& cb) :
            cb_(cb) { }
        void run() { cb_(); }
    private:
        std::function<void ()> cb_;
};

// Schduler dispatches work items to thread pool to run
class Scheduler
{
    public:
        Scheduler(const std::string& name, int numThread = DEFAULT_THREAD_NUM);
        ~Scheduler();
        void enqueueWorkItem(const WorkItem& workItem);
        WorkItem getWorkItem();
    private:
        std::string name_;
        std::queue<WorkItem> workItems_;
        std::vector<Thread*> threads_;
        std::mutex mutex_;
        std::condition_variable cv_;
};

}} //namespace
#endif

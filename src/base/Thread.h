#ifndef __THREAD_H__
#define __THREAD_H__

#include <unistd.h>

#include <iostream>
#include <thread>
#include <map>

#include "Log.h"
#include "Singleton.h"

namespace yux{
namespace base{

class Scheduler;

class Thread
{
    public:
        Thread(const std::string& name, Scheduler *pScheduler = nullptr);
        virtual ~Thread();

        size_t getId() { return id_; }
        std::thread::id getTid() { return tid_; }
        std::string& getName() { return name_; }
        inline std::thread::id start();
        void stop() { stop_  = true; }
        void join() { thr_->join(); }
        void detach() { thr_->detach(); }
        virtual void didStart() { }
    protected:
        std::string     name_;
        size_t          id_;     // For thread manager
        std::thread::id tid_;
        std::thread     *thr_;
        bool            stop_;
        Scheduler       *pScheduler_;
        void threadEntry();
        virtual void workBody();
};

class ThreadManager : public Singleton<ThreadManager>
{
    friend class Singleton<ThreadManager>;

    public:
        size_t addThread(Thread* pThr);
        void dropThread(Thread* pThr);
        void dump();
    private:
        ThreadManager() : nextId_(0) {}
        ~ThreadManager();
        std::mutex mutex_;
        std::map<uint32_t, Thread*> threadTable_;
        size_t nextId_;
};

// Use inline here so libBase.so will not need to link pthread
// pthread is needed only when Thread is really used.
std::thread::id Thread::start()
{
    if (id_)
    {
        // Start work thread
        thr_ = new std::thread(&Thread::threadEntry, this);
        tid_ = thr_->get_id();
        log_trace("Thread %d started - Tid: %x ..", id_, tid_);
    }
    else
    {
        // Main thread if id_== 0;
        workBody();
        tid_ = std::this_thread::get_id();
    }

    return tid_;
}

}}
#endif

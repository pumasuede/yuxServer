#ifndef __THREAD_H__
#define __THREAD_H__

#include <unistd.h>

#include <iostream>
#include <thread>
#include <map>

#include "Log.h"

namespace yux{
namespace base{

class Thread
{
protected:
    std::string     name_;
    size_t          id_;     // For thread manager
    std::thread::id tid_;
    std::thread     *thr_;
    Thread          *parent_;
    bool            stop_;
    void threadEntry();

public:
    Thread(const std::string& name, Thread *parent = NULL);
    virtual ~Thread();

    size_t getId() { return id_; }
    std::thread::id getTid() { return tid_; }
    std::string& getName() { return name_; }

    inline std::thread::id start();
    void stop() { stop_  = true; }
    void join() { thr_->join(); }
    void detach() { thr_->detach(); }

    virtual void didStart() { }
    virtual void workBody();
};

class ThreadManager
{
    std::mutex mutex_;
    std::map<uint32_t, Thread*> threadTable_;
    size_t nextId_;

public:
    ThreadManager() : nextId_(0) {}
    ~ThreadManager();
    size_t addThread(Thread* pThr);
    void dropThread(Thread* pThr);
    void dump();
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

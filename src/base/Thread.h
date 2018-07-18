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
    std::thread     *thr_;
    Thread          *parent_;
    bool            stop_;
    void threadEntry();

public:
    Thread(const std::string& name, Thread *parent = NULL);
    virtual ~Thread() { delete thr_; }
    inline std::thread::id start();
    inline void stop();
    void join() { thr_->join(); }
    void datach() { thr_->detach(); }
    std::thread::id getTid() { return thr_->get_id(); }
    std::string& getName() { return name_; }

    virtual void didStart() {};
    virtual void workBody();
};

class ThreadManager
{
    std::mutex mutex_;
    std::map<uint32_t, Thread*> threadTable_;
    size_t next_id_;
    static ThreadManager m_;
    ThreadManager() : next_id_(0) {}

public:
    static ThreadManager* getInstance();
    size_t addThread(Thread* pThr);
    size_t dropThread(Thread* pThr) { return 0; }
    void dump();
};

std::thread::id Thread::start()
{
    std::thread::id tid;
    if (id_)
    {
        // Start work thread
        thr_ = new std::thread(&Thread::threadEntry, this);
        tid = thr_->get_id();
    }
    else
    {
        // Main thread if id_== 0;
        workBody();
        tid = std::this_thread::get_id();
    }

    return tid;
}

inline void Thread::stop()
{
    stop_ = true;
}

}}
#endif

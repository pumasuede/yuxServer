#include <iostream>
#include <map>
#include <unistd.h>
#include <pthread.h>

#include "Mutex.h"
#include "Log.h"

#ifndef __THREAD_H__
#define __THREAD_H__

class Thread
{
protected:
    std::string    name_;
    pthread_t      tid_;
    Thread         *parent_;
    size_t         id_;       // for thread manager
    bool           stop_;

    static void* threadEntry(void* t);
public:
    Thread(const std::string& name, Thread *parent = NULL);
    virtual ~Thread() {}
    inline pthread_t start();
    inline void stop();
    virtual void didStart() {};
    virtual void workBody();

    pthread_t getTid() { return tid_; }
    std::string& getName() { return name_; }
};

class ThreadManager
{
    Mutex mutable mutex_;
    std::map<uint32_t, Thread*> threadTable_;
    size_t next_id_;
    static ThreadManager m_;
    ThreadManager() : next_id_(0) {}

public:
    static ThreadManager* getInstance();
    size_t addThread(Thread& t);
    size_t dropThread(Thread& t) { return 0; }
    void dump();
};

pthread_t Thread::start()
{
    if (id_)
    {
        // start work thread
        pthread_create(&tid_, NULL, &Thread::threadEntry, this);
    }
    else
    {
        // main thread id_= 0;
        tid_ = pthread_self();
        workBody();
    }

    return tid_;
}

inline void Thread::stop()
{
    stop_ = true;
}

#endif

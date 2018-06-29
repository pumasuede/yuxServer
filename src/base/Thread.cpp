#include "Thread.h"

using namespace std;

ThreadManager ThreadManager::m_;
ThreadManager* ThreadManager::getInstance() { return &m_; }

void* Thread::threadEntry(void* t)
{
    Thread* thread = static_cast<Thread*>(t);

    if (thread)
    {
        thread->didStart();
        thread->workBody();
    }

    return NULL;
}

void Thread::workBody()
{
    int cnt = 0;
    while (!stop_)
    {
        log_debug("---workd body: %s Tid 0x%x wake times : %d", name_.c_str(), getTid(), cnt++);
        usleep(cnt/3*1000*100);
    }
}

Thread::Thread(const std::string& name, Thread *parent ) : name_(name), parent_(parent), stop_(false)
{
    id_ = ThreadManager::getInstance()->addThread(this);
    cout<<"Thread "<<id_ <<" is created..\n";
}


// Thread manager
size_t ThreadManager::addThread(Thread *pThr)
{
    Lock lock(mutex_);
    if ( next_id_>65535 )
        next_id_=0;
    threadTable_[next_id_] = pThr;
    return next_id_++;
}

void ThreadManager::dump()
{
    std::map<uint32_t, Thread*>::iterator it;
    for (auto& thr : threadTable_)
        cout<<"*** Thread "<< thr.first<<" -> "<<thr.second->getName()<<" id : "<<it->second->getTid()<<"\n";
}

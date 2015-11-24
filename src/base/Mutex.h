#include <pthread.h>

#ifndef __MUTEXT_H__
#define __MUTEXT_H__

class CondVar;
class Mutex
{
    public:
        Mutex() { pthread_mutex_init(&mutex_, NULL); }
        ~Mutex() { pthread_mutex_destroy(&mutex_); }
        void lock() { pthread_mutex_lock(&mutex_); }
        void unlock() { pthread_mutex_unlock(&mutex_); }
    private:
        friend class CondVar;
        Mutex(const Mutex&);
        Mutex& operator=(const Mutex&);
        pthread_mutex_t mutex_;
};


class Lock
{
    public:
        Lock(Mutex& m) : m_(m) { m_.lock();}
        ~Lock() { m_.unlock(); }
    private:
        Mutex& m_;

};

#endif

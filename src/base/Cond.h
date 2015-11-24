#include "Mutex.h"

#ifndef __COND_H_H_
#define __COND_H_H_

class CondVar
{
    public:
        explicit CondVar(Mutex & mutex);
        ~CondVar();
        void wait();
        void signal();
        void broadcast();
    private:
        CondVar(const CondVar&);
        CondVar& operator=(const CondVar&);
        pthread_cond_t condvar_;
        Mutex& mutex_;

};

#endif

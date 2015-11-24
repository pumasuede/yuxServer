#include "Cond.h"

CondVar::CondVar(Mutex& mutex) : mutex_(mutex)
{
    pthread_cond_init(&condvar_, NULL);
}

CondVar::~CondVar()
{
    pthread_cond_destroy(&condvar_);
}

void CondVar::wait()
{
    pthread_cond_wait(&condvar_, &mutex_.mutex_);
}

void CondVar::signal()
{
    pthread_cond_signal(&condvar_);
}

void CondVar::broadcast()
{
    pthread_cond_broadcast(&condvar_);
}

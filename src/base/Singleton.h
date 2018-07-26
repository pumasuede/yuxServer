#ifndef __YUX_SINGLETON_H_
#define __YUX_SINGLETON_H_

#include <iostream>
#include <pthread.h>

namespace yux {
namespace base {

template <class T>
class Singleton
{
    public:
        static T* getInstance();
    protected:
        Singleton() {} ;
    private:
        Singleton(const T&) = delete;
        Singleton &operator=(const T&) = delete;
};

template <class T>
T* Singleton<T>::getInstance()
{
    static T obj;
    return &obj;
}

}} //namespace
#endif

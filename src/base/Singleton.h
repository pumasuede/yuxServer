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
    private:
        Singleton(const T&);
        Singleton &operator=(const T&);
        Singleton();
};

template <class T>
T* Singleton<T>::getInstance()
{
    static T obj;
    return &obj;
}

}} //namespace
#endif

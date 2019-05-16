#ifndef __YUX_SINGLETON_H_
#define __YUX_SINGLETON_H_

#include <iostream>

namespace yux {
namespace base {

template <class T>
class Singleton
{
    public:
        static T* getInstance();
        Singleton(const Singleton<T>&) = delete;
        Singleton& operator=(const Singleton<T>&) = delete;
    protected:
        Singleton() {} ;
};

template <class T>
T* Singleton<T>::getInstance()
{
    static T obj;
    return &obj;
}

}} //namespace
#endif

#ifndef _YUX_OBSERVABLE_H_
#define _YUX_OBSERVABLE_H_

#include <set>

namespace yux {
namespace base {

// T is an observer inferface which defines the call back functions on specific envents
// Use template here so we can use custom observer class which can define various call back interfaces.
template <class T>
class Observable
{
    public:
        virtual void addObserver(T* pObserver) {  observers_.insert(pObserver); }
        virtual void removeObserver(T* pObserver) { observers_.erase(pObserver); }
        virtual ~Observable();
    protected:
        std::set<T*> observers_;
};

template <class T>
Observable<T>::~Observable()
{
    for (auto ob : observers_)
    {
        // Check if obsever is self. If so, no need to delete it.
        Observable* p = dynamic_cast<Observable*>(ob);
        if (p != nullptr && p == this)
        {
            delete ob;
        }
    }
}

}} // name space
#endif

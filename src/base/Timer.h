#ifndef __YUX_TIMER_H__
#define __YUX_TIMER_H__

#include <sys/time.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <functional>

namespace yux {
namespace base {

struct Timer
{
    typedef std::function<int (void*)> TimerCallBack;
    Timer(int mSec, const TimerCallBack& timerCb): mSec_(mSec), timerCb_(timerCb), lastFired_(-1) { }

    int mSec_;
    int lastFired_;
    TimerCallBack timerCb_;
};

std::vector<Timer>& getTimers();

bool regTimer(const Timer::TimerCallBack& cb, int mSec);

}}//namespace


#define DECL_TIMER(name, msec) \
    int name(void*); \
    bool unused##name = yux::base::regTimer(name, msec);
#endif

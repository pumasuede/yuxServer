#ifndef __YUX_UTILS_H__
#define __YUX_UTILS_H__

#include <sys/time.h>

#include <string>
#include <tr1/functional>

namespace yux{
namespace base{

struct Timer
{
    typedef std::tr1::function<int (void*)> TimerCallBack;
    Timer(int mSec, TimerCallBack timerCb): mSec_(mSec), timerCb_(timerCb), lastFired_(-1) { }

    int mSec_;
    int lastFired_;
    TimerCallBack timerCb_;
};

std::string GetFormatTime();

}}//namespace

#endif

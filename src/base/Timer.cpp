#include "Timer.h"

namespace yux {
namespace base {

std::vector<Timer> g_Timers;
std::vector<Timer>& getTimers()
{
    return g_Timers;
}

bool regTimer(Timer::TimerCallBack cb, int mSec)
{
    Timer timer(mSec, cb);
    g_Timers.push_back(timer);
}

}}

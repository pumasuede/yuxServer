#ifndef __YUX_UTILS_H__
#define __YUX_UTILS_H__

#include <sys/time.h>
#include <stdlib.h>

#include <string>
#include <functional>

namespace yux{
namespace base{

struct Timer
{
    typedef std::function<int (void*)> TimerCallBack;
    Timer(int mSec, TimerCallBack timerCb): mSec_(mSec), timerCb_(timerCb), lastFired_(-1) { }

    int mSec_;
    int lastFired_;
    TimerCallBack timerCb_;
};

std::string GetFormatTime();

struct URLParser{
    URLParser(const std::string& url) : url_(url), port_(0) { }
    void parse();
    std::string url_;
    std::string proto_;
    std::string host_;
    uint16_t    port_;
    std::string path_;
    std::string query_;

};

}}//namespace

#endif

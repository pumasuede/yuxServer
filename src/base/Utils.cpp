#include "Utils.h"
#include <stdio.h>

namespace yux{
namespace base{

using namespace std;

string GetFormatTime()
{
    time_t time;
    struct timeval tv;
    struct tm *tm;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    tm = localtime(&time);
    char buf[1024];

    int len = sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec/1000));
    if (len < 0)
    {
        return "";
    }

    return string(buf);
}

}} //name space

#include "Utils.h"
#include <stdio.h>

namespace yux{
namespace base{

using namespace std;

void URLParser::parse()
{
    if (url_.empty())
        return;

    int findOffset = 0;

    size_t protoEnd = url_.find("://");

    // proto can be ignored.
    if (protoEnd != string::npos)
    {
        proto_ = url_.substr(0, protoEnd);
        findOffset = protoEnd +3;
    }

    size_t hostEnd = url_.find("/", findOffset);
    if (hostEnd == string::npos)
    {
        hostEnd = url_.size();
    }

    // port can be ignored
    size_t portStart = url_.find(":", findOffset);
    if (portStart != string::npos)
    {
        port_ = atoi(url_.substr(portStart+1, hostEnd - portStart).c_str());
        host_ = url_.substr(findOffset, portStart - findOffset);
    }
    else
    {
        host_ = url_.substr(findOffset, hostEnd-findOffset);
        port_ = 80; //default port
    }
    findOffset = hostEnd;

    // parameters can be ignored
    size_t queryStart = url_.find("?", findOffset);
    if (queryStart != string::npos)
    {
        path_ = url_.substr(findOffset, queryStart - findOffset);
        query_ = url_.substr(queryStart+1);
    }
    else
    {
        path_ = url_.substr(findOffset);
    }
}

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

string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');

    if (first == string::npos)
    {
        return str;
    }

    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

}} //name space

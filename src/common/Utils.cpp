#include "Utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <string>

namespace yux {
namespace common {

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

void writeNum(unsigned char* dest, unsigned int number, int bytes)
{
    if( 1 == bytes )
    {
        *dest = static_cast<unsigned char>(number & 0x7f);
    }
    else if( 2 == bytes )
    {
        *dest = static_cast<unsigned char>((number >> 8) & 0xff);
        *(dest + 1) = static_cast<unsigned char>(number & 0xff);
    }
    else if( 4 == bytes )
    {
        *dest = static_cast<unsigned char>((number >> 24) & 0xff);
        *(dest + 1) = static_cast<unsigned char>((number >> 16) & 0xff);
        *(dest + 2) = static_cast<unsigned char>((number >> 8) & 0xff);
        *(dest + 3) = static_cast<unsigned char>(number & 0xff);
        *dest |= 0x80;
    }
}

int getFileSize(const std::string& filename)
{
     struct stat stat_buf;
     int rc = stat(filename.c_str(), &stat_buf);
     return rc == 0 ? stat_buf.st_size : -1;
}

bool isBinary(const std::string& contentType)
{
    return contentType.find("text/") == string::npos;
}

void hexDump(const char* ptr, int buflen, int fd)
{
    unsigned char *buf = (unsigned char*) ptr;
    int i, j, n;
    char str[128];
    for (i=0; i < buflen; i+=16)
    {
        int n = sprintf(str, "%06x: ", i);
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                n += sprintf(str + n , "%02x ", buf[i+j]);
            else
                n += sprintf(str + n, "   ");
        n += sprintf(str + n, " |");
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                n += sprintf(str + n, "%c", isprint(buf[i+j]) ? buf[i+j] : '.');
        n += sprintf(str + n, "\n");
        write(fd, str, n);
    }
}

}} //name space

#include <ctype.h>
#include <iostream>

#include "HttpParser.h"

namespace yux{
namespace parser{

bool HttpLineParser::parse(HttpRequest &req, const char *buf, uint32_t size)
{
    std::string line(buf, size);
    std::cout<<"htpp parse line:\n"<<line<<"\n";
    return true;
}

bool HttpRequestParser::parse(HttpRequest &req, const char *buf, uint32_t size)
{
    std::cout<<"ready to parse http...\n";
    int i=0, begin=0;

    while (i<size)
    {
        if (i==begin && !isalpha(buf[i]))
        {
            i++;
            begin = i;
            continue;
        }

        if (buf[i]=='\r' && buf[i+1]=='\n')
        {
            if (!lineParser_.parse(req, buf+begin, i-begin))
            {
                error_ = lineParser_.parseError();
                return false;
            }

            i += 2;
            begin = i;
        }
        else
        {
            i++;
        }
    }

    return true;
}

}} //name space

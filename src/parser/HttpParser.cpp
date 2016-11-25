#include <ctype.h>
#include <iostream>

#include "HttpParser.h"

using namespace std;

namespace yux{
namespace parser{

bool HttpLineParser::parse(HttpRequest &req, const char *buf, uint32_t size)
{
    string line(buf, size);
    size_t pos = 0;
    if ((pos=line.find("GET")) != string::npos)
    {
        size_t pos_s=0, pos_e=0;
        pos_s = line.find("/", pos);
        pos_e = line.find(" HTTP/", pos_s);
        if (pos_e > pos_s)
        {
            req.URI = line.substr(pos_s, pos_e-pos_s);
        }
        std::cout<<"parsed URI:"<<req.URI<<"\n";
    }
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

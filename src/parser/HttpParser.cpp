#include <ctype.h>
#include <iostream>
#include <regex>

#include "HttpParser.h"

using namespace std;

namespace yux{
namespace parser{

bool HttpReqLineParser::isStartLine()
{
    regex reg("(GET|PUT|POST|DELETE) .* HTTP/\\d{1}\\.\\d{1}");
    return regex_match(lineBuf_, reg) ? true : false;
}

HttpReqStartLine HttpReqLineParser::parseStartLine()
{
    HttpReqStartLine startLine;
    std::stringstream lineStream(lineBuf_);
    lineStream>>startLine.method>>startLine.URI>>startLine.version;
    return startLine;
}

void HttpLineParser::parseOption(string& name, string& value)
{
}

bool HttpReqLineParser::parse(HttpRequest& req)
{
    if (isStartLine())
    {
        req.startLine = parseStartLine();
    }
    else
    {
        string name, value;
        parseOption(name, value);
        req.header[name] = value;
    }

    return true;
}

bool HttpRequestParser::parse(HttpRequest& req, const char *buf, uint32_t size)
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
            HttpReqLineParser lineParser(buf+begin, i-begin);
            if (!lineParser.parse(req))
            {
                error_ = lineParser.error();
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

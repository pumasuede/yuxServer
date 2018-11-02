#include <ctype.h>
#include <iostream>
#include <regex>

#include "HttpParser.h"
#include "base/Log.h"

using namespace std;
using namespace yux::base;

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

    const string& uri = startLine.URI;
    int posQueryString = uri.find_last_of("?");
    bool hasQueryString = posQueryString != string::npos;

    startLine.scriptName = hasQueryString ? uri.substr(0, posQueryString): uri;
    startLine.queryString = hasQueryString ? uri.substr(posQueryString+1) : "";

    return startLine;
}

void HttpLineParser::parseOption(string& name, string& value)
{
    int sep = lineBuf_.find(':');
    if (sep != string::npos)
    {
        name = lineBuf_.substr(0, sep);
        value = lineBuf_.substr(sep+2); //skip the space after :
    }

    log_trace("Parsed header line: %s | %s", name.c_str(), value.c_str());
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
    bool isBody = false;

    while (i<size)
    {
        if (buf[i]=='\r' && buf[i+1]=='\n')
        {
            if (begin == i)
            {
                // The line has only '\r\n', which means it's the boundary between header and body.
                isBody = true;
                req.body = std::move(string(buf+i+2, size-i-2));
                log_trace("Parsed Body:\n%s", req.body.c_str());
                return true;
            }

            // Parse line;
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

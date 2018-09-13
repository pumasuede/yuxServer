#ifndef HTTP_PARSER_H__
#define HTTP_PARSER_H__

#include <stdint.h>
#include <string>
#include <map>

namespace yux
{
namespace parser
{

struct HttpReqStartLine
{
    std::string method;
    std::string URI;
    std::string version;
};

struct HttpRequest
{
    HttpReqStartLine startLine;
    std::map<std::string, std::string> header;
    std::map<std::string, std::string> postData;
};

class HttpLineParser
{
    public:
        HttpLineParser(const char *begin, uint32_t size) : lineBuf_(begin, size) { }
        virtual ~HttpLineParser() { }
        virtual bool isStartLine() = 0;
        virtual void parseOption(std::string& name, std::string& value);
        std::string& error() { return error_; }
    protected:
        std::string lineBuf_;
        std::string error_;
};

class HttpReqLineParser : public HttpLineParser
{
    public:
        // parse a line.  return true if parse sucessfully
        HttpReqLineParser(const char *begin, uint32_t size) : HttpLineParser(begin, size) { }
        virtual bool isStartLine() override;
        virtual HttpReqStartLine parseStartLine();
        bool parse(HttpRequest& req);
};

class HttpRequestParser
{
    public:
        bool parse(HttpRequest& req, const char *begin, uint32_t size);
        std::string& error() {return error_; }

    private:
        std::string error_;
};

}} //name space
#endif

#ifndef HTTP_PARSER_H__
#define HTTP_PARSER_H__

#include <string>
#include <map>

namespace yux
{
namespace parser
{

typedef struct HttpRequest
{
    std::string method;
    std::string version;
    std::string URI;
    std::map<std::string, std::string> header;

} HttpRequest;

class HttpLineParser
{
    public:
    bool parse(HttpRequest &req, const char *begin, uint32_t size);
    std::string& parseError() { return error_; }
    private:
    std::string error_;
};

class HttpRequestParser
{
    public:
        bool parse(HttpRequest &req, const char *begin, uint32_t size);
        std::string& parseError() {return error_; }

    private:
        HttpLineParser lineParser_;
        std::string error_;

        inline bool isControl(int c)
        {
            return (c >= 0 && c <= 31) || (c == 127);
        }

        // Check if a byte is a digit.
        inline bool isDigit(int c)
        {
            return c >= '0' && c <= '9';
        }
};

}} //name space

#endif

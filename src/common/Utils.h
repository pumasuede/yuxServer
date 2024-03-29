#ifndef __YUX_UTILS_H__
#define __YUX_UTILS_H__

#include <sys/time.h>
#include <stdlib.h>

#include <string>
#include <functional>

namespace yux {
namespace common {

std::string getFormatTime();
std::string trim(const std::string& str);
void writeNum(unsigned char* dest, unsigned int number, int bytes);
int getFileSize(const std::string& filename);
bool isBinary(const std::string& fileExt);
void hexDump(const char* ptr, int buflen, int fd = 1);

// Check if a char is a control char.
inline bool isControl(char c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

// Check if a char is a digit.
inline bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

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

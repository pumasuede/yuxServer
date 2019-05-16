#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <thread>
#include <mutex>

#include "Singleton.h"

#ifndef _YUX_LOG_H__
#define _YUX_LOG_H__

namespace yux{
namespace base{

class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;
    public:
        enum LogLevel
        {
            LEVEL_NONE    = -1,
            LEVEL_MIN     = 0,
            LEVEL_FATAL   = 0,
            LEVEL_ERROR   = 1,
            LEVEL_WARN    = 2,
            LEVEL_INFO    = 3,
            LEVEL_DEBUG   = 4,
            LEVEL_TRACE   = 5,
            LEVEL_MAX     = 5,
        };

        static int getLevel(const std::string& levelname);

    private:
        Logger();
        ~Logger();

        std::string fileName_;
        int fd_;
        int level_;
        std::mutex mutex_;

        uint64_t rotateSize_;

        struct{
            uint64_t w_curr;
            uint64_t w_total;
        } stats_;

        void rotate();
    public:
        int fd() { return fd_; }
        int level() { return level_; }
        void set_level(int level) { level_ = level;}

        int open(int fd, int level=LEVEL_DEBUG);
        int open(const std::string& filename, int level=LEVEL_DEBUG, uint64_t rotate_size=0);
        void close();

        int logv(int level, const char *fmt, va_list ap);

        int fatal(const char *fmt, ...);
        int error(const char *fmt, ...);
        int warn(const char *fmt, ...);
        int info(const char *fmt, ...);
        int debug(const char *fmt, ...);
        int trace(const char *fmt, ...);
};

inline int log_open(int fd, int level=Logger::LEVEL_DEBUG) { return Logger::getInstance()->open(fd, level); }
inline int log_open(const char *filename, int level=Logger::LEVEL_DEBUG, uint64_t rotateSize=0) { return Logger::getInstance()->open(filename, level, rotateSize); }

inline int log_level() { return Logger::getInstance()->level(); }
inline void set_log_level(int level) { Logger::getInstance()->set_level(level); }

int log_write(int level, const char *fmt, ...);
std::string getClassMethodName(std::string&& prettyFunction);

#define __FUNC_STR__ getClassMethodName(std::string(__PRETTY_FUNCTION__)).c_str()
#define __THREAD_ID__ std::this_thread::get_id()

#ifdef NDEBUG
#define log_trace(fmt, args...) do{}while(0)
#else
#define log_trace(fmt, args...) \
    log_write(yux::base::Logger::LEVEL_TRACE, "[%x](%d)[%s]: " fmt, __THREAD_ID__,  __LINE__, __FUNC_STR__, ##args)
#endif

#define log_debug(fmt, args...) \
    log_write(Logger::LEVEL_DEBUG, "[%x][%s] " fmt, __THREAD_ID__, __FUNC_STR__, ##args)
#define log_info(fmt, args...) \
    log_write(Logger::LEVEL_INFO, "[%x][%s] " fmt, __THREAD_ID__, __FUNC_STR__, ##args)
#define log_warn(fmt, args...) \
    log_write(Logger::LEVEL_WARN, "[%x][%s] " fmt, __THREAD_ID__, __FUNC_STR__, ##args)
#define log_error(fmt, args...) \
    log_write(Logger::LEVEL_ERROR, "[%x][%s] " fmt, __THREAD_ID__, __FUNC_STR__, ##args)
#define log_fatal(fmt, args...) \
    log_write(Logger::LEVEL_FATAL, "[%x][%s] " fmt, __THREAD_ID__, __FUNC_STR__, ##args)

#define log_trace_hexdump(buf, size) \
    if (yux::base::Logger::getInstance()->level() >= Logger::LEVEL_TRACE ) \
        yux::common::hexDump(buf, size, yux::base::Logger::getInstance()->fd())

#define log_hexdump(buf, size) \
    yux::common::hexDump(buf, size, yux::base::Logger::getInstance()->fd())
}}
#endif

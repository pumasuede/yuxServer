#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

class Logger{
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

        static int get_level(const char *levelname);
    private:
        FILE *fp;
        char filename[PATH_MAX];
        int level_;
        pthread_mutex_t mutex_;

        uint64_t rotate_size;
        struct{
            uint64_t w_curr;
            uint64_t w_total;
        }stats;

        void rotate();
    public:
        Logger();
        ~Logger();

        static Logger logger_;

        int level(){
            return level_;
        }

        void set_level(int level){
            this->level_ = level;
        }

        int open(FILE *fp, int level=LEVEL_DEBUG);
        int open(const char *filename, int level=LEVEL_DEBUG, uint64_t rotate_size=0);
        void close();

        int logv(int level, const char *fmt, va_list ap);

        int trace(const char *fmt, ...);
        int debug(const char *fmt, ...);
        int info(const char *fmt, ...);
        int warn(const char *fmt, ...);
        int error(const char *fmt, ...);
        int fatal(const char *fmt, ...);
};


int log_open(FILE *fp, int level=Logger::LEVEL_DEBUG);
int log_open(const char *filename, int level=Logger::LEVEL_DEBUG, uint64_t rotate_size=0);
int log_level();
void set_log_level(int level);
int log_write(int level, const char *fmt, ...);


#ifdef NDEBUG
#define log_trace(fmt, args...) do{}while(0)
#else
#define log_trace(fmt, args...)	\
    log_write(Logger::LEVEL_TRACE, "%s(%d): " fmt, __FILE__, __LINE__, ##args)
#endif

#define log_debug(fmt, args...)	\
    log_write(Logger::LEVEL_DEBUG, "[%x][%s - %s] " fmt, pthread_self(), __FILE__, __func__, ##args)
#define log_info(fmt, args...)	\
    log_write(Logger::LEVEL_INFO, "[%x][%s - %s] " fmt, pthread_self(), __FILE__, __func__, ##args)
#define log_warn(fmt, args...)	\
    log_write(Logger::LEVEL_WARN, "[%x][%s - %s] " fmt, pthread_self(), __FILE__, __func__, ##args)
#define log_error(fmt, args...)	\
    log_write(Logger::LEVEL_ERROR, "[%x][%s - %s] " fmt, pthread_self(), __FILE__, __func__, ##args)
#define log_fatal(fmt, args...)	\
    log_write(Logger::LEVEL_FATAL, "[%x][%s - %s] " fmt, pthread_self(), __FILE__, __func__, ##args)
#endif

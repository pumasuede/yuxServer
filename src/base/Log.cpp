#include "Log.h"

Logger Logger::logger_;

int log_open(FILE *fp, int level ){
    return Logger::logger_.open(fp, level);
}

int log_open(const char *filename, int level, uint64_t rotate_size){
    return Logger::logger_.open(filename, level, rotate_size);
}

int log_level(){
    return Logger::logger_.level();
}

void set_log_level(int level){
    Logger::logger_.set_level(level);
}

int log_write(int level, const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = Logger::logger_.logv(level, fmt, ap);
    va_end(ap);
    return ret;
}

/*****/

Logger::Logger(){
    fp = stdout;
    level_ = LEVEL_DEBUG;

    filename[0] = '\0';
    rotate_size = 0;
    stats.w_curr = 0;
    stats.w_total = 0;
    pthread_mutex_init(&mutex_, NULL);
}

Logger::~Logger(){
    pthread_mutex_destroy(&mutex_);
    this->close();
}

int Logger::open(FILE *fp, int level){
    this->fp = fp;
    this->level_ = level;

    return 0;
}

int Logger::open(const char *filename, int level, uint64_t rotate_size){
    if(strlen(filename) > PATH_MAX - 20){
        fprintf(stderr, "log filename too long!");
        return -1;
    }
    strcpy(this->filename, filename);

    FILE *fp;
    if(strcmp(filename, "stdout") == 0){
        fp = stdout;
    }else if(strcmp(filename, "stderr") == 0){
        fp = stderr;
    }else{
        fp = fopen(filename, "a");
        if(fp == NULL){
            return -1;
        }

        struct stat st;
        int ret = fstat(fileno(fp), &st);
        if(ret == -1){
            fprintf(stderr, "fstat log file %s error!", filename);
            return -1;
        }else{
            this->rotate_size = rotate_size;
            stats.w_curr = st.st_size;
        }
    }
    return this->open(fp, level);
}

void Logger::close(){
    if(fp != stdin && fp != stdout){
        fclose(fp);
    }
}

void Logger::rotate(){
    fclose(fp);
    char newpath[PATH_MAX];
    time_t time;
    struct timeval tv;
    struct tm *tm;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    tm = localtime(&time);
    sprintf(newpath, "%s.%04d%02d%02d-%02d:%02d:%02d-%d",
            this->filename,
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec);

    printf("rename %s => %s\n", this->filename, newpath);
    int ret = rename(this->filename, newpath);
    if(ret == -1){
        fprintf(stderr, "rename %s error!", filename);
        return;
    }
    fp = fopen(this->filename, "a");
    if(fp == NULL){
        return;
    }
    stats.w_curr = 0;
}

int Logger::get_level(const char *levelname){
    if(strcmp("trace", levelname) == 0){
        return LEVEL_TRACE;
    }
    if(strcmp("debug", levelname) == 0){
        return LEVEL_DEBUG;
    }
    if(strcmp("info", levelname) == 0){
        return LEVEL_INFO;
    }
    if(strcmp("warn", levelname) == 0){
        return LEVEL_WARN;
    }
    if(strcmp("error", levelname) == 0){
        return LEVEL_ERROR;
    }
    if(strcmp("fatal", levelname) == 0){
        return LEVEL_FATAL;
    }
    return LEVEL_DEBUG;
}

inline static const char* level_name(int level){
    switch(level){
        case Logger::LEVEL_FATAL:
            return "[FATAL] ";
        case Logger::LEVEL_ERROR:
            return "[ERROR] ";
        case Logger::LEVEL_WARN:
            return "[WARN ] ";
        case Logger::LEVEL_INFO:
            return "[INFO ] ";
        case Logger::LEVEL_DEBUG:
            return "[DEBUG] ";
        case Logger::LEVEL_TRACE:
            return "[TRACE] ";
    }
    return "";
}

#define LEVEL_NAME_LEN	8
#define LOG_BUF_LEN		4096

int Logger::logv(int level, const char *fmt, va_list ap)
{
    if(logger_.level_ < level){
        return 0;
    }

    char buf[LOG_BUF_LEN];
    int len;
    char *ptr = buf;

    time_t time;
    struct timeval tv;
    struct tm *tm;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    tm = localtime(&time);

    len = sprintf(ptr, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec/1000));
    if(len < 0){
        return -1;
    }
    ptr += len;

    memcpy(ptr, level_name(level), LEVEL_NAME_LEN);
    ptr += LEVEL_NAME_LEN;

    int space = sizeof(buf) - (ptr - buf) - 10;
    len = vsnprintf(ptr, space, fmt, ap);
    if(len < 0){
        return -1;
    }
    ptr += len > space? space : len;
    *ptr++ = '\n';
    *ptr = '\0';

    len = ptr - buf;
    // change to write(), without locking?
    fwrite(buf, len, 1, this->fp);
    fflush(this->fp);

    stats.w_curr += len;
    stats.w_total += len;

    if (rotate_size > 0)
    {
        pthread_mutex_lock(&mutex_);
        if( stats.w_curr > rotate_size)
        {
            const char *p = "begin to rotate";
            fwrite(p, strlen(p), 1, this->fp);
            rotate();
        }
        pthread_mutex_unlock(&mutex_);

        return len;
    }

    return 0;
}

int Logger::trace(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_TRACE, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::debug(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_DEBUG, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::info(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_INFO, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::warn(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_WARN, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::error(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_ERROR, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::fatal(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int ret = logger_.logv(Logger::LEVEL_FATAL, fmt, ap);
    va_end(ap);
    return ret;
}

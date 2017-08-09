#include "Log.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

int log_write(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = Logger::instance().logv(level, fmt, ap);
    va_end(ap);
    return ret;
}

Logger::Logger() : fd_(-1), level_(LEVEL_DEBUG), rotateSize_(0)
{
    stats_.w_curr = 0;
    stats_.w_total = 0;
    pthread_mutex_init(&mutex_, NULL);
}

Logger::~Logger()
{
    pthread_mutex_destroy(&mutex_);
    ::close(fd_);
}

int Logger::open(int fd, int level)
{
    fd_ = fd;
    level_ = level;

    struct stat st;
    int ret = fstat(fd_, &st);
    if (ret == -1)
    {
        fprintf(stderr, "fstat log file %s error!", filename_.c_str());
        return -1;
    }

    stats_.w_curr = st.st_size;

    return 0;
}

int Logger::open(const std::string& filename, int level, uint64_t rotateSize)
{
    if (filename.size() > PATH_MAX - 20)
    {
        fprintf(stderr, "log filename too long!");
        return -1;
    }

    filename_ = filename;
    rotateSize_ = rotateSize;

    int fd;

    if (filename == "stdout")
    {
        fd = 1;
    }
    else if (filename == "stderr")
    {
        fd = 2;
    }
    else if (access(filename.c_str(), F_OK) != -1)
    {
        fd = ::open(filename_.c_str(), O_RDWR|O_APPEND);
    }
    else
    {
        fd = ::open(filename_.c_str(), O_RDWR|O_CREAT, 0755);
    }

    if (fd == -1)
    {
        fprintf(stderr, "Can't open log file:%s\n", filename_.c_str());
        return -1;
    }

    return open(fd, level);
}

void Logger::close(){
     ::close(fd_);
}

void Logger::rotate()
{
    ::close(fd_);
    char newPath[PATH_MAX];
    time_t time;
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    tm = localtime(&time);
    sprintf(newPath, "%s.%04d%02d%02d-%02d:%02d:%02d-%d",
            filename_.c_str(),
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec);
    printf("rename %s => %s\n", filename_.c_str(), newPath);

    int ret = rename(filename_.c_str(), newPath);
    if(ret == -1)
    {
        fprintf(stderr, "rename %s error!", filename_.c_str());
        return;
    }

    fd_ = ::open(filename_.c_str(), O_RDWR|O_APPEND);
    if (fd_ = -1)
    {
        return;
    }
    stats_.w_curr = 0;
}

int Logger::getLevel(const std::string& levelname)
{
    if (levelname == "trace")
    {
        return LEVEL_TRACE;
    }

    if (levelname == "debug")
    {
        return LEVEL_DEBUG;
    }

    if (levelname == "info")
    {
        return LEVEL_INFO;
    }

    if (levelname == "error")
    {
        return LEVEL_ERROR;
    }

    if (levelname == "fatal")
    {
        return LEVEL_FATAL;
    }

    if (levelname == "warn")
    {
        return LEVEL_WARN;
    }

    return LEVEL_DEBUG;
}

inline static const char* level_name(int level)
{
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
    if (level_ < level)
    {
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
    if (len < 0)
    {
        return -1;
    }
    ptr += len;

    memcpy(ptr, level_name(level), LEVEL_NAME_LEN);
    ptr += LEVEL_NAME_LEN;

    int space = sizeof(buf) - (ptr - buf) - 10;
    len = vsnprintf(ptr, space, fmt, ap);
    if(len < 0)
    {
        return -1;
    }
    ptr += len > space? space : len;
    *ptr++ = '\n';
    *ptr = '\0';

    len = ptr - buf;
    ::write(fd_, buf, len);

    stats_.w_curr += len;
    stats_.w_total += len;

    if (rotateSize_ > 0)
    {
        if( stats_.w_curr > rotateSize_)
        {
            const char *p = "begin to rotate";
            ::write(fd_, p, strlen(p));
            pthread_mutex_lock(&mutex_);
            rotate();
            pthread_mutex_unlock(&mutex_);
        }

        return len;
    }

    return 0;
}

int Logger::trace(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_TRACE, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::debug(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_DEBUG, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_INFO, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_WARN, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_ERROR, fmt, ap);
    va_end(ap);
    return ret;
}

int Logger::fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = logv(Logger::LEVEL_FATAL, fmt, ap);
    va_end(ap);
    return ret;
}

#ifndef DC_BASE_LOGGING_H
#define DC_BASE_LOGGING_H
#include "logStream.h"
#include "timeStamp.h"

namespace dc
{
class TimeZone;
class Logger
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };

    class SourceFile
    {
    public:
        // compile time calculation of basename of source file
        template<int N>
        SourceFile(const char (&arr)[N]) : m_data(arr), m_size(N - 1)
        {
            const char* slash = strrchr(m_data, '/' ); // return the postion of '/'
            if(slash)
            {
                m_data = slash + 1;
                m_size -= static_cast<int>(m_data - arr);
            }
        }

        explicit SourceFile(const char* filename) : m_data( filename )
        {
            const char* slash = strrchr(m_data, '/');
            if(slash)
            {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
        }

        const char* m_data;
        int m_size;
    private:

    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level) ;
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return m_impl.m_stream; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);
    using OutputFunc = void (*)(const char* msg, int len);
    using FlushFunc = void (*)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setTimeZone(const TimeZone& tz);


private:
    class Impl
    {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();
        Timestamp m_time;
        LogStream m_stream;
        LogLevel m_level;
        int m_line;
        SourceFile m_basename;
    };

    Impl m_impl;
};

extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}
/*
    this expands to
    if(good)
        if(logging_INFO)
            logInfoStream << "Good news";
        else
            logInfoStream << "Bad news";
*/
#define LOG_TRACE if (dc::Logger::logLevel() <= dc::Logger::TRACE) \
    dc::Logger(__FILE__, __LINE__, dc::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (dc::Logger::logLevel() <= dc::Logger::DEBUG) \
    dc::Logger(__FILE__, __LINE__, dc::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (dc::Logger::logLevel() <= dc::Logger::INFO ) \
    dc::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN dc::Logger(__FILE__, __LINE__, dc::Logger::WARN).stream()
#define LOG_ERROR dc::Logger(__FILE__, __LINE__, dc::Logger::ERROR).stream()
#define LOG_FATAL dc::Logger(__FILE__, __LINE__, dc::Logger::FATAL).stream()
#define LOG_SYSERR dc::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL dc::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
//check that the input is non NULL, This very useful in constructor
//initializer lists.
#define CHECK_NOTNULL(val) \
    ::dc::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr)
{
    if(ptr == nullptr)
    {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}
}
#endif
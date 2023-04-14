#include "logging.h"
#include "currentThread.h"
#include "timeZone.h"
#include "timeStamp.h"

#include <errno.h>
#include <cstdio>
#include <sstream>

namespace dc
{
class TimeZone;
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno,t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
    if(getenv("DC_LOG_TRACE"))
        return Logger::TRACE;
    else if(getenv("DC_LOG_DEBUG"))
        return Logger::DEBUG;
    else    
        return Logger::INFO;
}
Logger::LogLevel g_logLevel = initLogLevel();
const char* LogLevelName[Logger::NUM_LOG_LEVELS] = 
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  " ,
    "ERROR ",
    "FATAL ",
};

// helper class for know string length at complie time
class T
{
public:
    T(const char* str, unsigned len) : m_str(str), m_len(len)
    {
        //printf("m_len = %d, str = %s, strlen(str) == %ld \n", m_len, str, strlen(str));
        assert(strlen(str) == m_len);
    }
    const char* m_str;
    const unsigned m_len;
};

inline LogStream& operator<<(LogStream& s, T v)
{
    s.append(v.m_str, v.m_len);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.append(v.m_data, v.m_size);
    return s;
}

void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
TimeZone g_logTimeZone;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
    : m_time(Timestamp::now()),
      m_stream(),
      m_level(level),
      m_line(line),
      m_basename(file)
{
    formatTime();
    CurrentThread::tid();
    m_stream << T(CurrentThread::tidString(), CurrentThread::tidStringLength()) << ' ';
    m_stream << T(LogLevelName[level], 6);
    if(savedErrno != 0)
    {
        m_stream << strerror_tl(savedErrno) << " (errno =" << savedErrno << ") ";
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = m_time.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch/ Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if( seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct DateTime dt;
        if( g_logTimeZone.valid())
        {
            dt = g_logTimeZone.toLocalTime(seconds);
        }
        else
        {
            dt = TimeZone::toUtcTime(seconds);
        }
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d", 
                        dt.m_year,dt.m_month, dt.m_day, dt.m_hour, dt.m_minute, dt.m_second);
        assert(len == 17);
        (void)len;
    }

    if(g_logTimeZone.valid())
    {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8 );
        m_stream << T(t_time, 17) << T(us.data(), 8);
    }
    else
    {
        Fmt us(".%06dz ", microseconds);
        assert(us.length() == 9);
        m_stream << T(t_time, 17) << T(us.data(), 9);
    }
}

void Logger::Impl::finish()
{
    m_stream << " - " << m_basename << ':' << m_line << '\n';
}

Logger::Logger(SourceFile file, int line) : m_impl(INFO, 0, file, line)
{

}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func) : m_impl(level, 0, file, line)
{
    m_impl.m_stream << func <<' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level) : m_impl(level, 0, file, line)
{

}

Logger::Logger(SourceFile file, int line, bool toAbort) : m_impl(toAbort? FATAL: ERROR, errno, file, line)
{

}

Logger::~Logger()
{
    m_impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if(m_impl.m_level == FATAL)
    {
        printf("%s\n%s\n", "fatal error", strerror_tl(errno));
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}

void Logger::setTimeZone(const TimeZone& tz)
{
    g_logTimeZone = tz;
}

} // namespace dc

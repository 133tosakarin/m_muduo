#include "logFile.h"
#include "processInfo.h"
#include <assert.h>
#include <cstdio>
#include <time.h>


namespace dc
{

LogFile::LogFile(const string& basename, 
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN ) :
	m_basename(basename), m_rollSize(rollSize),
    m_flushInterval(flushInterval),
    m_checkEveryN(checkEveryN),
    m_count(0),
    m_mutex(threadSafe ? new MutexLock : nullptr),
    m_startOfPeriod(0),
    m_lastRoll(0),
    m_lastFlush(0)
{
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
    if(m_mutex)
    {
        MutexLockGuard lock(*m_mutex);
        append_unlocked(logline, len);
    }
    else
    {
        append_unlocked(logline, len);
    }
}

void LogFile::flush()
{
    if(m_mutex)
    {
        MutexLockGuard lock(*m_mutex);
        m_file->flush();
    }
    else
    {
        m_file->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len)
{
    m_file->append(logline, len);
    if(m_file->writtenBytes() > rollFile())
    {
        rollFile();
    }
    else
    {
        ++m_count;
        if( m_count > m_checkEveryN)
        {
            m_count = 0;
            time_t now = ::time(nullptr);
            time_t thisPeriod = now / _kRollPerSeconds * _kRollPerSeconds;
            if( thisPeriod != m_startOfPeriod )
            {
                rollFile();
            }
            else if( now - m_lastFlush > m_flushInterval )
            {
                m_lastFlush = now;
                m_file->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    string filename = getLogFileName(m_basename, &now);
    time_t start = now / _kRollPerSeconds * _kRollPerSeconds;
    if( now > m_lastRoll )
    {
        m_lastRoll = now;
        m_lastFlush = now;
        m_startOfPeriod = start;
        m_file.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now)
{
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;
    char timebuf[32];
    struct tm tm;
    *now = time(nullptr);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%H%S.", &tm);
    filename +=ProcessInfo::hostname();
    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ProcessInfo::pid());

    filename +=".log";
    return filename;
}
}

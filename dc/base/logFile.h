#ifndef DC_BASE_LOGFILE_H
#define DC_BASE_LOGFILE_H

#include "mutex.h"
#include "types.h"
#include "fileUtil.h"
#include <memory>

namespace dc
{
class AppendFile;
class LogFile : noncopyable
{
public:
    LogFile(const string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();
private:
    void append_unlocked(const char* logline, int len);

    static string getLogFileName(const string& basename, time_t* now);

    const string m_basename;// log file name
    const off_t m_rollSize;// when logfile' size lager or eaqual to rollsize will change a new file.
    const int m_flushInterval;//the interval that write log time 
    const int m_checkEveryN;

    int m_count;
    std::unique_ptr<dc::MutexLock> m_mutex;
    time_t m_startOfPeriod;//start record log time(call time)
    time_t m_lastRoll;//last roll logfile time
    time_t m_lastFlush;//last time that write log to file
    std::unique_ptr<dc::AppendFile> m_file;

    const static int _kRollPerSeconds = 60 * 60 *24;
};
}
#endif

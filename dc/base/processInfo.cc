#include "processInfo.h"
#include "currentThread.h"
#include "fileUtil.h"

#include <algorithm>
#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
namespace dc
{


thread_local int t_numOpenedFiles = 0;
int fdDirFilter(const struct dirent* d)
{
    if( ::isdigit(d->d_name[0]))
    {
        ++t_numOpenedFiles;
    }
    return 0;
}

__thread std::vector<pid_t>* t_pids = nullptr;

int taskDirFilter(const struct dirent* d)
{
    if(isdigit(d->d_name[0]))
    {
        t_pids->push_back(atoi(d->d_name));
    }
    return 0;
}

int scanDir(const char* dirpath, int (*filter)(const struct dirent *))
{
    struct dirent** namelist = nullptr;
    int ret = ::scandir(dirpath, &namelist, filter, alphasort);
    assert(namelist == nullptr);
    return ret;
}
Timestamp g_startTime = Timestamp::now();

// assume those won't change during the life time of a process

// sysconf get the config information of system, this func is get  tick numbers of clock per second
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
/*
_SC_CHILD_MAX：每个user可同时运行的最大进程数
_SC_HOST_NAME_MAX：hostname最大长度，需小于_POSIX_HOST_NAME_MAX (255)
_SC_OPEN_MAX：一个进程可同时打开的文件最大数
_SC_PAGESIZE：一个page的大小，单位byte
_SC_PHYS_PAGES：物理内存总page数
_SC_AVPHYS_PAGES：当前可获得的物理内存page数
_SC_NPROCESSORS_CONF：配置的处理器个数
_SC_NPROCESSORS_ONLN：当前可获得的处理器个数
_SC_CLK_TCK：每秒对应的时钟tick数
*/
int g_pageSize = static_cast<int>(::sysconf(_SC_PHYS_PAGES));

namespace ProcessInfo
{
pid_t pid()
{
    return getpid();
}
std::string pidString()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", pid());
    return buf;
}
uid_t uid()
{
    return getuid();
}
std::string username()
{
    struct passwd pwd;
    struct passwd* ret = nullptr;
    char buf[8192];
    const char* name = "unknownuser";
    getpwuid_r(uid(), &pwd, buf, sizeof(buf), &ret);
    if( ret )
    {
        name = pwd.pw_name;
    }
    return name;

}
uid_t euid()
{
    return ::geteuid();
}
Timestamp startTime()
{
    return g_startTime;
}
int clockTicksPerSecond()
{
    return g_clockTicks;
}
int pageSize()
{
    return g_pageSize;
}
bool isDebugBuild()
{
    #ifdef NDEBUG
        return false;
    #else
        return true;
    #endif
}
std::string hostname()
{
    char buf[256];
    if( ::gethostname(buf, sizeof(buf)) == 0)
    {
        buf[sizeof(buf) - 1] = 0;
        return buf;
    }
    else
    {
        return "unknownhost";
    }
}
std::string procname()
{
    return procname(procStat()).as_string();
}
StringPiece procname(const std::string& stat)
{
    StringPiece name;
    size_t lp = stat.find('(');
    size_t rp = stat.find(')');
    if( lp != string::npos && rp != string::npos && lp < rp)
    {
        name.set(stat.data() + lp + 1, static_cast<int>(rp - lp + 1));
    }
    return name;
}


// read /proc/self/status
std::string procStatus()
{
    string ret;
    readFile("/proc/self/status", 65536, &ret);
    return ret;
}

// read /proc/self/stat
std::string procStat()
{
    string ret;
    readFile("/proc/self/stat", 65536, &ret);
    return ret;
}



// read /proc/self/task/tid/stat
std::string threadStat()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "proc/self/task/%d/stat", CurrentThread::tid());
    string ret;
    readFile(buf, 65536, &ret);
    return ret;
}

// readlink /proc/self/exe

std::string exePath()
{
    string ret;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
    if( n > 0 )
        ret.assign(buf, n);
    return ret;
}
int openedFiles()
{
    t_numOpenedFiles = 0;
    scanDir("/proc/self/fd", fdDirFilter);
    return t_numOpenedFiles;
}
int maxOpenFiles()
{
    struct rlimit rl;
    if( ::getrlimit(RLIMIT_NOFILE, &rl))
    {
        return openedFiles();
    }
    else
    {
        return static_cast<int>(rl.rlim_cur);
    }
}

CpuTime cpuTime()
{
    CpuTime cputime;
    struct tms tms;
    if(::times(&tms) >= 0)
    {
        const double hz = static_cast<double>(clockTicksPerSecond());
        cputime.userSeconds = static_cast<double>(tms.tms_utime) /hz;
        cputime.systemSeconds = static_cast<double>(tms.tms_stime) /hz;

    }
    return cputime;
}

int numThreads()
{
    int ret = 0;
    string status = procStatus();
    size_t pos = status.find("Threads:");
    if( pos != string::npos)
    {
        ret = ::atoi(status.c_str() + pos + 8);
    }
    return ret;
}

std::vector<pid_t> threads()
{
    std::vector<pid_t> ret;
    t_pids = &ret;
    scanDir("/proc/self/tack", taskDirFilter);
    t_pids = nullptr;
    std::sort(ret.begin(), ret.end());
    return ret;
}

std::vector<pid_t> threads();
}

}
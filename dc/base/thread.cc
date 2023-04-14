#include "thread.h"
#include "currentThread.h"
#include "exception.h"
#include "logging.h"

#include <type_traits>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <cstdio>
#include <sys/types.h>
#include <errno.h>
namespace dc
{

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
    CurrentThread::t_cachedTid = 0;
    CurrentThread::t_threadName = "main";
    CurrentThread::tid();
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer()
    {
        dc::CurrentThread::t_threadName = "main";
        CurrentThread::tid();
        pthread_atfork(nullptr, nullptr, &afterFork);
    }
};

ThreadNameInitializer init;

struct ThreadData
{
    using ThreadFunc = Thread::ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch* latch_;
    ThreadData(ThreadFunc func, 
               const std::string& name,
               pid_t* tid, 
               CountDownLatch* latch) : func_(func), name_(name), tid_(tid), latch_(latch)
    {

    }

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        CurrentThread::t_threadName = name_.empty() ? "dcThread" : name_.c_str();
        ::prctl(PR_SET_NAME, CurrentThread::t_threadName); // set process name
        try
        {
            func_();
        }
        catch(const Exception& e)
        {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", e.what());
            fprintf(stderr, "stack trace: %s\n", e.stackTrace());
            abort();
        }
        catch(const std::exception& e)
        {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread: %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", e.what());
            abort();
        }
        catch(...)
        {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw;
        }
        
    }
};

void* startThread( void* obj )
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}

void CurrentThread::cachedTid()
{
    if( t_cachedTid == 0 )
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
    return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts = {0, 0};
    ts.tv_sec = static_cast<time_t>( usec/ Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
    ::nanosleep(&ts, nullptr);
}

AtomicInt32 Thread::m_numCreated;
Thread::Thread( ThreadFunc func, const std::string& name ) : is_started(false), 
                                                             is_joined(false), 
                                                             m_threadId(0), 
                                                             m_pid(0), 
                                                             m_func(func), 
                                                             m_name(name), 
                                                             m_latch(1)
{
    setDefaultName();
}
//FixME: make it movable in c++11
Thread::~Thread()
{
    if( is_started && !is_joined )
    {
        pthread_detach(m_threadId);
    }
}

void Thread::start()
{
    assert(!is_started);
    is_started = true;
    ThreadData* data = new ThreadData(m_func, m_name, &m_pid, &m_latch);
    if( pthread_create(&m_threadId, nullptr, &startThread, data))
    {
        is_started = false;
        delete data;
        LOG_SYSFATAL << "Failed in pthread_create";
    }
    else
    {
        m_latch.wait();
        assert(m_pid > 0 );
    }
}

int Thread::join()
{
    assert(is_started);
    assert(!is_joined);
    is_joined = true;
    return pthread_join(m_threadId, nullptr);
}

void Thread::setDefaultName()
{
    int num = m_numCreated.incrementAndGet();
    if(m_name.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d ", num);
        m_name = buf;
    }
}

}

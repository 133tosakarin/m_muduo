#ifndef DC_BASE_THREADPOOL_H
#define DC_BASE_THREADPOOL_H

#include "condition.h"
#include "mutex.h"
#include "thread.h"
#include "types.h"
#include "logging.h"
#include <deque>
#include <vector>

namespace dc
{

class ThreadPool : noncopyable
{
public:
    using Task = std::function<void ()>;
private:
    bool isFull() const REQUIRES(m_mutex);
    void runInThread();
    Task take();
public:
    explicit ThreadPool(const string& nameArg = string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { _maxQueueSize = maxSize; }
    void setThreadInitCallback(const Task& cb) { _threadInitCallback = cb; }

    void start(int numThreads);
    void stop();
    const string& name() const
    {
        return m_name;
    }
    size_t queueSize() const;
    /*
        Could block if maxQueueSize > 0
        call after  stop() will return immediately
        There is no move-only version of std::function in c++ as of c++14
        so we don't need to overload a const& and an && versions
        as we do in (Bounded)BlockingQueue.

    */
   void run(Task f);
private:
    mutable MutexLock m_mutex;
    Condition _notEmpty GUARDED_BY(m_mutex);
    Condition _notFull GUARDED_BY(m_mutex);
    string m_name;
    Task _threadInitCallback;
    std::vector<std::unique_ptr<dc::Thread>> m_threads;
    std::deque<Task> m_queue GUARDED_BY(m_mutex);
    size_t _maxQueueSize;
    bool is_running;
};
}
#endif

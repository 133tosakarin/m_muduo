#ifndef DC_BASE_ASYNCLOGGING_H
#define DC_BASE_ASYNCLOGGING_H

#include "thread.h"
#include "logStream.h"
#include "mutex.h"
#include <vector>
#include <memory>
#include <atomic>
namespace dc
{
class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const string& basename, 
                 off_t rollSize,
                 int flushInterval = 3);

    ~AsyncLogging()
    {
        if(is_running)
        {
            stop();
        }
    }

    void append(const char* logline, int len);

    void start()
    {
        is_running = true;
        m_thread.start();
        m_latch.wait();
    }

    void stop() //NO_THREAD_SAFETY_ANALYSIS
    {
        is_running = false;
        m_cond.notify();
        m_thread.join();
    }

private:
    void threadFunc();
    using Buffer = dc::FixedBuffer<dc::kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    const int m_flushInterval;
    std::atomic<bool> is_running;
    const string m_basename;
    const off_t m_rollSize;
    dc::Thread m_thread;
    dc::CountDownLatch m_latch;
    dc::MutexLock m_mutex;
    dc::Condition m_cond GUARDED_BY(m_mutex);
    BufferPtr m_currentBuffer GUARDED_BY(m_mutex);
    BufferPtr m_nextBuffer GUARDED_BY(m_mutex);
    BufferVector m_buffers GUARDED_BY(m_mutex);

};
}
#endif
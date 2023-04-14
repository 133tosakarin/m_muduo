#include "threadpool.h"
#include "exception.h"
#include <assert.h>
#include <cstdio>
namespace dc
{

ThreadPool::ThreadPool(const string& nameArg) : m_mutex(), _notEmpty(m_mutex), 
                                                _notFull(m_mutex), m_name(nameArg), _maxQueueSize(0), is_running(false)
{

}

ThreadPool::~ThreadPool()
{
    if(is_running)
    {
        stop();
    }
}

void ThreadPool::start(int  numThreads)
{
    assert(m_threads.empty());
    is_running = true;
    m_threads.reserve(numThreads);
    for (int  i = 0; i < numThreads; ++i )
    {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        m_threads.emplace_back(new dc::Thread(std::bind(&ThreadPool::runInThread, this), m_name + id));
        m_threads[i]->start();
    }
    if( numThreads == 0 && _threadInitCallback)
    {
        _threadInitCallback();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(m_mutex);
        is_running = false;
        _notEmpty.notifyAll();
        _notFull.notifyAll();
    }
    for (auto& thr : m_threads)
    {
        thr->join();
    }
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(m_mutex);
    return m_queue.size();
}

void ThreadPool::run(Task task)
{
    if(m_threads.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(m_mutex);
        while(isFull() && is_running)
        {
            _notFull.wait();
        }
        if( !is_running ) return;

        assert(!isFull());
        m_queue.push_back(std::move(task));
        _notEmpty.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(m_mutex);
    while(m_queue.empty() && is_running )
    {
        _notEmpty.wait();
    }
    Task task;
    if( !m_queue.empty())
    {
        task = m_queue.front();
        m_queue.pop_front();
        if( _maxQueueSize > 0)
        {
            _notFull.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const
{
    m_mutex.assertLocked();
    return _maxQueueSize > 0 && m_queue.size() >= _maxQueueSize;

}

void ThreadPool::runInThread()
{
    try
    {
        if(_threadInitCallback)
        {
            _threadInitCallback();
        }
        while(is_running)
        {
            Task task(take());
            if( task )
            {
                task();
            }
        }
    }
    catch(const dc::Exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        fprintf(stderr, "stack trace: %s\n", e.stackTrace());
        abort();
    }
    catch(const std::exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        abort();
    }
    catch(...)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        throw;
    }
    
}
}
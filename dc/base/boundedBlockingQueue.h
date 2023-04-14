#ifndef DC_BASE_BOUNDEDBLOCKINGQUEUE_H
#define DC_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "condition.h"
#include "mutex.h"
#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace dc
{

template<typename T>
class BoundedBlockingQueue : noncopyable
{
public:
    explicit BoundedBlockingQueue(int maxSize) : m_mutex(), m_notEmpty(m_mutex), m_notFull(m_mutex), m_queue(maxSize)
    {

    }

    void put(const T& x)
    {
        MutexLockGuard lcok(m_mutex);
        while(m_queue.full())
        {
            m_notFull.wait();
        }

        assert(!m_queue.full());
        m_queue.push_back(x);
        m_notEmpty.notify();
    }

    void put(T&& x)
    {
        MutexLockGuard lock(m_mutex);
        while(m_queue.full())
        {
            m_notFull.wait();
        }
        assert(!m_queue.full());
        m_queue.push_back(std::move(x));
        m_notEmpty.notify();
    }

    T take()
    {
        MutexLockGuard lock(m_mutex);
        while(m_queue.empty())
        {
            m_notEmpty.wait();
        }
        assert(!m_queue.empty());
        T front(std::move(m_queue.front()));
        m_queue.pop_front();
        m_notFull.notify();
        return front;
    }

    bool empty() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.empty();
    }

    bool full() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.full();
    }

    size_t size() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.size();
    }

    size_t capacity() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.capacity();
    }

private:
    mutable MutexLock m_mutex;
    Condition m_notEmpty GUARDED_BY(m_mutex);
    Condition m_notFull GUARDED_BY(m_mutex);
    boost::circular_buffer<T> m_queue GUARDED_BY(m_mutex);

};

}

#endif

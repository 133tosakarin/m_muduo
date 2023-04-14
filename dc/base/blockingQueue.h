#ifndef DC_BASE_BLOCKINGQUEUE_H
#define DC_BASE_BLOCKINGQUEUE_H
#include "condition.h"
#include "mutex.h"

#include <deque>
#include <assert.h>
namespace dc
{

template<typename T>
class BlockingQueue : noncopyable
{

public:
    using queue_type = std::deque<T>;
    BlockingQueue() : m_mutex(), m_notEmpty(m_mutex), m_queue()
    {

    }
    void put(const T& x)
    {
        MutexLockGuard lock(m_mutex);
        m_queue.push_back(x);
        m_notEmpty.notify(); // wait morphing saves us
    }
	void put(T&& x)
	{
		MutexLockGuard lock(m_mutex);
		m_queue.emplace_back(std::move(x));
		m_notEmpty.notify();
	}
    T take()
    {
        MutexLockGuard lock(m_mutex);
        while(m_queue.empty())
        {
            m_notEmpty.wait();//always use a while-loop, due to spurious wakeup
        }
        assert(!m_queue.empty());
        T front(std::move(m_queue.front()));
        m_queue.pop_front();
        return front;
    }

    queue_type drain()
    {
        std::deque<T> queue;
        {
            MutexLockGuard lock(m_mutex);
            queue = std::move(m_queue);
            assert(m_queue.empty());
        }
        return queue;
    }
    size_t size() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.size();
    }
private:
    mutable MutexLock m_mutex;
    Condition m_notEmpty GUARDED_BY(m_mutex);
    queue_type m_queue GUARDED_BY(m_mutex);
};

}

#endif

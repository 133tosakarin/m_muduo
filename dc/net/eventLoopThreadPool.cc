#include "dc/net/eventLoopThreadPool.h"
#include "dc/net/eventLoop.h"
#include "dc/net/eventLoopThread.h"

#include <cstdio>
using namespace dc;
using namespace dc::net;


EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, const string& nameArg)
	: m_baseLoop(loop),
	  m_name(nameArg),
	  is_started(false),
	  m_numThreads(0),
	  m_next(0)
{

}
EventLoopThreadPool::~EventLoopThreadPool()
{

}
void EventLoopThreadPool::start(const ThreadInitCallback& cb )
{
	assert(!is_started);
	m_baseLoop->assertInLoopThread();
	is_started = true;
	for( int i = 0; i < m_numThreads; ++i )
	{
		char buf[m_name.size() + 32];
		snprintf(buf, sizeof buf, "%s%d", m_name.c_str(), i);
		EventLoopThread* t = new EventLoopThread(cb, buf);
		m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
		m_loops.push_back(t->startLoop());
	
	}
	if( m_numThreads == 0 && cb )
	{
		cb(m_baseLoop);
	}
}


EventLoop* EventLoopThreadPool::getNextLoop()
{
	m_baseLoop->assertInLoopThread();
	assert(is_started);
	EventLoop* loop = m_baseLoop;
	if( !m_loops.empty())
	{
		loop = m_loops[m_next];
		++m_next;
		if(implicit_cast<size_t>(m_next) >= m_loops.size())
		{
			m_next = 0;
		}
	}
	return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
	m_baseLoop->assertInLoopThread();
	EventLoop* loop = m_baseLoop;
	if(!m_loops.empty())
	{
		loop = m_loops[hashCode % m_loops.size()];
	}
	return loop;
}


std::vector<EventLoop* > EventLoopThreadPool::getAllLoops()
{
	m_baseLoop->assertInLoopThread();
	assert(is_started);
	if( m_loops.empty())
	{
		return std::vector<EventLoop*>(1, m_baseLoop);
	}
	else
	{
		return m_loops;
	}
}

#include "dc/net/eventLoop.h"
#include "dc/base/logging.h"
#include "dc/net/eventLoopThread.h"


using namespace dc;
using namespace dc::net;

EventLoopThread::EventLoopThread( const ThreadInitCallback& cb,
					 const string& name )
	: m_loop(nullptr),
	 is_exiting(false),
	 m_thread(std::bind(&EventLoopThread::threadFunc, this), name),
	 m_mutex(),
	 m_cond(m_mutex),
	 m_callback(cb)
{


}
EventLoopThread::~EventLoopThread()
{
	is_exiting = true;
	if( m_loop != nullptr )
	{
		m_loop->quit();
		m_thread.join();
	}


}
EventLoop* EventLoopThread::startLoop()
{
 assert(!m_thread.started());
 m_thread.start();
 EventLoop* loop = nullptr;
 {
	 MutexLockGuard lock(m_mutex);
	 while( m_loop == nullptr )
	 {
		 m_cond.wait();
	 }
	loop = m_loop; 
 }
 return loop;

}
void EventLoopThread::threadFunc()
{
	EventLoop loop;
	//LOG_INFO << "loop addr = " << &loop;
	if( m_callback)
	{
		m_callback(&loop);
	}

	{
		MutexLockGuard lock(m_mutex);
		m_loop = &loop;
		m_cond.notify();
	}
	loop.loop();
	MutexLockGuard lock(m_mutex);
	m_loop = nullptr;


}


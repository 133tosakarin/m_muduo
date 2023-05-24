/*
 * =====================================================================================
 *
 *       Filename:  eventLoopThread.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2023 11:30:48 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef DC_NET_EVENTLOOPTHREAD_H
#define DC_NET_EVENTLOOPTHREAD_H

#include "dc/base/thread.h"
#include "dc/base/mutex.h"
#include "dc/base/condition.h"


namespace dc
{
namespace net
{
class EventLoop;

class EventLoopThread : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;
	EventLoopThread( const ThreadInitCallback& cb = ThreadInitCallback(),
					 const string& name = string());
	~EventLoopThread();
	EventLoop* startLoop();


private:
	void threadFunc();
	EventLoop* m_loop GUARDED_BY(m_mutex);
	bool is_exiting;
	Thread m_thread;
	MutexLock m_mutex;
	Condition m_cond GUARDED_BY(m_mutex);
	ThreadInitCallback m_callback;
};
}//namespace net
}//namespace dc
#endif

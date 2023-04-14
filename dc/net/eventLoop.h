#ifndef DC_NET_EVENTLOOP_H
#define DC_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>
#include <any>

#include "dc/base/noncopyable.h"
#include "dc/base/mutex.h"
#include "dc/base/timeStamp.h"
#include "dc/net/timerId.h"
#include "dc/net/callbacks.h"
#include "dc/base/currentThread.h"

namespace dc
{
namespace net
{


class Channel;
class Poller;
class TimerQueue;


//Reactor, at most one per thread
class EventLoop : noncopable
{
public:
	using std::function<void()> Functor;
	EventLoop();
	~EventLoop();

	// must be called in the same thread as
	// creation of the object.
	void loop();

	//this is not 100% thread safe, if you 
	//call through a raw pointer, better to 
	//call through shared_ptr<EventLoop> for 100% safety.
	void quit();

	Timestamp pollReturnTime() const { return m_pollReturnTime; }

	int64_t iteration() const { return iteration; } 

	// Runs callback immediately in the loop thread.
	// It wakeup the loop , and run the cb.
	// If in the same loop thread, cb is run withing the function
	// safe to call from other threads
	void runInLoop(Functor cb);

	// Queue callback in the loop thread.
	// Runs after finish polling.
	// safe to call from other threads.
	void queueInLoop(Functor cb);

	size_t queueSize() const;

	// timers
	// Runs callback at 'time'

	void runAt(Timestamp time, TimerCallback cb);

	void runAfter(double delay, TimerCallback cb);

	void runEvery(double interval, TimerCallback cb);


	void cancel(TimerId timerId);

	// Channel
	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	void assertInLoopThread();
	{
		if(!isInLoopThread())
		{
			abortNotInLoopThread();
		}

	}
	bool isInLoopThread() const
	{
		return m_threadId == CurrentThread::tid(); 
	}
	
	bool eventHandling() const
	{
		return is_eventHandling;
	}

	void setContext(const std::any& context)
	{
		m_context = context;
	}

	std::any* getMutableContext() 	{
		return &m_context;
	}
	std::any& getContext() const
	{
		return m_context;
	}

	static EventLoop* getEventLoopOfCurrentThread();
private:
	using ChannelList = std::vector<Channel*>;
	void abortNotInThread();
	void handleRead();
	void doPendingFunctors();
	
	void printActiveChannels() const;
	
	std::atomic<bool> is_looping;
	std::atomic<bool> is_quit;
	std::atomic<bool> is_eventHandling;
	std::atomic<bool> is_callingPendingFunctors;
	int64_t iteration;
	const pid_t m_threadId;
	Timestamp m_pollReturnTIme;
	std::unique_ptr<Poller> m_poller;
	std::unique_ptr<TimerQueue> m_timerQueue;
	int wakeupFd;
	std::unique_ptr<Channel> m_wakeupChannel;
	std::any m_context;

	ChannelList m_activeChannels;
	Channel* currentActiveChannel;

	mutable MutexLock m_mutex;
	std::vector<Functor> m_pendingFunctors GUARDED_BY(m_mutex);
};

} //namespace net
}// namespace dc



#endif //DC_NET_EVENTLOOP_H

#ifndef DC_NET_EVENTLOOP_H
#define DC_NET_EVENTLOOP_H
#include "dc/base/mutex.h"
#include "dc/base/currentThread.h"
#include "dc/base/timeStamp.h"
#include "dc/net/timerId.h"
#include "dc/net/callbacks.h"

#include <functional>
#include <vector>
#include <atomic>
#include <any>

namespace dc
{
namespace net
{

class EventLoop : noncopyable
{
public:
	using Functor = std::function<void()>;
	
	EventLoop();
	~EventLoop(); // force out-line dtor, for std::unique_ptr members.

	//loops forever
	//must be called in the same thread as creation of the object
	void loop();

	//This is not 100% thread safe, if ou call through a raw pointer,
	//better to call through shared_ptr<EventLoop> for 100% safety
	void quit();

	//time when poll returns , usually means data arrival.
	Timestamp pollReturnTime() const { return m_pollReturnTime; }
	
	int64_t iteration() const { return m_iteration; }

	// runs callback immediately in the loop thread.
	// it wakes up the loop, and run the cb
	// if in the same loop thread, cb is run within the function.
	// safe to call from other threads
	
	void runInLoop(Functor cb);

	//queues callback in the loop thread.
	//runs after finish polling.
	//safe to call from other threads.
	//
	void queueInLoop(Functor cb);

	size_t queueSize() const;

	// Runs callback at 'time'.
	// safe to call from other threads.
	TimerId runat(Timestamp time, TimerCallback cb);

	//Runs callback after @c delay seconds
	//safe to call from other threads
	TimerId runAfter(double delay, TimerCallback cb);

	//Runs callback every @c interval seconds.
	//Safe to call from other threads.
	TimerId runEvery(double interval, TimerCallback cb);

	//cancels the timer
	void cancel(TimerId timerId);

	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	// pid_t threadId() const { return m_threadId; }
	//
	void assertInLoopThread()
	{
		if(!isLoopThread())
		{
			abortNotInLoopThread();
		}
	}
	
	bool isInLoopThread() const { return m_threadId == CurrentThread::tid(); }

	bool eventHandling() const { return is_eventHandling.load(); }
	bool setContext(const std::any& context)
	{
		m_context = context;
	}

	std::any* getMutableContext()
	{
		return &m_context;
	}

	static EventLoop* getEventLoopOfCurrentThread();
private:
	void abortNotInLoopThread();
	void handleRead(); //waked up
	void doPendingFunctors();

	void printActiveChannels() const; //DEBUG
	
	using ChannelList = std::vector<Channel*>;
	std::atomic<bool> is_looping;
	std::atomic<bool> is_quit;
	std::atomic<bool> is_eventHandling;
	std::atomic<bool> is_callingPendingFunctors;
	int64_t m_iteration;
	const pid_t m_threadId;
	Timestamp m_pollReturnTime;
	std::unique_ptr<Poller> m_poller;
	std::unique_ptr<TimerQueue> m_timerQueue;
	int m_wakeupFd;
	std::unique_ptr<Channel> m_wakeupChannel;
	std::any m_context;
	
	ChannelList m_activeChannels;
	Channel* m_currentActiveChannel;
	mutable MutexLocal m_mutex;
	std::vector<Functor> m_pendingFunctors GUAREDED_BY(m_mutex);

};

}//net

}//dc


#endif

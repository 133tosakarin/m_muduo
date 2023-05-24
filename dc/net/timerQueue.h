#ifndef DC_NET_TIMERQUEUE_H
#define DC_NET_TIMERQUEUE_H



#include "dc/base/mutex.h"
#include "dc/base/timeStamp.h"
#include "dc/net/callbacks.h"
#include "dc/net/channel.h"

#include <set>
#include <vector>
namespace dc
{
namespace net
{
class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{
public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();

	TimerId addTimer(TimerCallback cb,
					 Timestamp when,
					 double interval);
	void cancel(TimerId timerId);
private:


	using TimerPtr = Timer*;
	using Entry = std::pair<Timestamp, Timer*>;
	using TimerList = std::set<Entry>;
	using ActiveTimer = std::pair<Timer*, int64_t>;
	using ActiveTimerSet = std::set<ActiveTimer>;
	//these memfunc only were called in their I/O thread.
	void addTimerInLoop(TimerPtr timer);
	void cancelInLoop(TimerId timerId);

	void handleRead();

	std::vector<Entry> getExpired(Timestamp now);

	void reset(const std::vector<Entry>& expired, Timestamp now);

	bool insert(TimerPtr timer);

	EventLoop* m_loop;
	const int m_timerfd;
	Channel m_timerfdChannel;
	TimerList m_timers;
	ActiveTimerSet m_activeTimers;
	bool  is_callingExpiredTimers;
	ActiveTimerSet m_cancelingTimers;

};


} //namespace net 
} //namespace dc
#endif

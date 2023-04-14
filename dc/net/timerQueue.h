#ifndef DC_NET_TIMERQUEUE_H
#define DC_NET_TIMERQUEUE_H



#include "dc/net/eventLoop.h"
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


	using TimerPtr = std::unique_ptr<Timer>;
	using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
	using TimerList = std::set<Entry>;
	using ActiveTimer = std::pair<std::unique_ptr<Timer>, int64_t>;
	using ActiveTimerSete = std::set<ActiveTimer>;
	void addTimerInLoop(TimerPtr timer);
	void cancelInLoop(TimerId timerId);

	void handleRead();

	std::vector<Entry> getExpired(Timestamp now);

	void reset(const std::vector<Entry>& expired, Timestamp now);

	bool reset(TimerPtr timer);

	EventLoop* m_loop;
	const int m_timerfd;
	Channel m_timerfdChannel;
	TimerList m_timers;
	ActiveTimerSet m_activeTimers;
	atomic<bool> is_callingExpiredTimers;
	ActiveTimerSet m_cancelingTimers;

};


} //namespace net 
} //namespace dc
#endif

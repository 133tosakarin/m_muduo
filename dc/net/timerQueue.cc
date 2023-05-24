#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include "dc/net/timerQueue.h"
#include "dc/base/logging.h"
#include "dc/net/timer.h"
#include "dc/net/timerId.h"
#include "dc/net/eventLoop.h"


#include <sys/timerfd.h>
#include <unistd.h>

namespace dc
{
namespace net
{
namespace detail
{

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

	if(timerfd < 0 )
	{
	
		LOG_SYSFATAL << "failed in timerfd_creates";	
	}
	return timerfd;


}

struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	if( microseconds < 100 )
	{
		microseconds = 100;	
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
	return ts;

}

void readTimerfd(int timerfd, Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
	LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
	if( n != sizeof howmany )
	{
		LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";	
	}

}

void resetTimerfd(int timerfd, Timestamp expiration)
{
	//wake up loop by timerfd_settime()
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memZero(&newValue,  sizeof(itimerspec));
	memZero(&oldValue, sizeof(itimerspec));
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if( ret )
	{
		LOG_SYSERR << "timerfd_settime()";	
	}

}


}//namespace detail


}//namespace net

}//namespace dc




using namespace dc;
using namespace dc::net;
using namespace dc::net::detail;

TimerQueue::TimerQueue(EventLoop* loop)
	: m_loop(loop),
	  m_timerfd(createTimerfd()),
	  m_timerfdChannel(loop, m_timerfd),
	  m_timers(),
	  is_callingExpiredTimers(false)
{
	//LOG_TRACE << "timerfd = " << m_timerfd;
	m_timerfdChannel.setReadCallback( std::bind(&TimerQueue::handleRead, this));
	m_timerfdChannel.enableReading();

}
TimerQueue::~TimerQueue()
{
	m_timerfdChannel.disableAll();
	m_timerfdChannel.remove();
	::close(m_timerfd);
	for(const Entry& timer: m_timers)
	{
		delete timer.second;	
	}

}

TimerId TimerQueue::addTimer(TimerCallback cb,
					 		Timestamp when,
					 		double interval)
{
	Timer* timer  = new Timer(std::move(cb), when, interval);
	//LOG_TRACE << " new timer : " << timer << " time = " << when.toString();
	m_loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());

}
void TimerQueue::cancel(TimerId timerId)
{
	m_loop->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));

}
void TimerQueue::addTimerInLoop(TimerPtr timer)
{
	m_loop->assertInLoopThread();
	bool earliestChanged = insert(timer);
	if(earliestChanged)
	{
		resetTimerfd(m_timerfd, timer->expiration());	
	}

}
void TimerQueue::cancelInLoop(TimerId timerId)
{
	m_loop->assertInLoopThread();
	assert(m_timers.size() == m_activeTimers.size());
	ActiveTimer timer(timerId.m_timer, timerId.m_sequence);
	ActiveTimerSet::iterator it = m_activeTimers.find(timer);
	if( it != m_activeTimers.end() )
	{
		size_t n = m_timers.erase(Entry(it->first->expiration(), it->first));
		assert(n==1); (void)n;
		delete it->first;
		m_activeTimers.erase(it);
	}
	else if(is_callingExpiredTimers)
	{
		m_cancelingTimers.insert(timer);
	}
	assert(m_timers.size() == m_activeTimers.size());
}

void TimerQueue::handleRead()
{
	m_loop->assertInLoopThread();
	Timestamp now(Timestamp::now());
	readTimerfd(m_timerfd, now);
	//LOG_TRACE << "tiemr expired to exec something" << m_activeTimers.size();
	std::vector<Entry> expired = getExpired(now); //RVO
	is_callingExpiredTimers = true;
	m_cancelingTimers.clear();
	for( const Entry& it :expired )
	{
		LOG_TRACE << it.first.toString();
		it.second->run();
	}
	is_callingExpiredTimers = false;
	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(m_timers.size() == m_activeTimers.size());
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	TimerList::iterator end = m_timers.lower_bound(sentry);
	assert( end != m_timers.begin() || now < end->first );
	std::copy(m_timers.begin(), end, back_inserter(expired));
	m_timers.erase(m_timers.begin(), end);
	for( const Entry& it : expired )
	{
		ActiveTimer timer(it.second, it.second->sequence());
		size_t n = m_activeTimers.erase(timer);
		assert(n == 1);
	}
	assert(m_timers.size() == m_activeTimers.size());
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;
	for( const Entry& it : expired)
	{
		ActiveTimer timer(it.second, it.second->sequence());
		if( it.second->repeat() 
				&& m_cancelingTimers.find(timer) == m_cancelingTimers.end())
		{
			it.second->restart(now);
			insert(it.second);
		}
		else
		{
			delete it.second;
		}
		

	}
	if( !m_timers.empty() )
	{
		nextExpire = m_timers.begin()->second->expiration();
	}
	if( nextExpire.valid())
	{
		resetTimerfd(m_timerfd, nextExpire);
	}
}

bool TimerQueue::insert(TimerPtr timer)
{
	m_loop->assertInLoopThread();
	assert(m_timers.size() == m_activeTimers.size() );
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	TimerList::iterator it = m_timers.begin();
	//LOG_INFO << "m_timers.size() = " << m_timers.size() << " , m_activeTimers.size() = " << m_activeTimers.size();
	if( it == m_timers.end() || when < it->first )
	{
		earliestChanged = true;
	}
	{

		std::pair<ActiveTimerSet::iterator, bool> result = m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
		assert(result.second); (void)result;
	}

	{

		std::pair<TimerList::iterator, bool> result = m_timers.insert(Entry(when, timer));
		assert(result.second); (void)result;
	}
	assert(m_timers.size() == m_activeTimers.size());
	return earliestChanged;
}


#ifndef DC_NET_TIMER_H
#define DC_NET_TIMER_H

#include  "dc/base/atomic.h"
#include "dc/base/timeStamp.h"
#include "dc/net/callbacks.h"


namespace dc
{
namespace net
{

// Internal class for timer event.

class Timer : noncopyable
{
public:
	Timer(TimerCallback cb, Timestamp when, double interval)
	  : m_callback(std::move(cb)),
	  	m_expiration(when),
		m_interval(interval),
		is_repeat(interval > 0.0),
		m_sequence(s_numCreated.incrementAndGet())
	{ }
	
	void run()
	{
		m_callback();
	}

	Timestamp expiration() const { return m_expiration; }
	bool repeat() const { return is_repeat; }
	void restart(Timestamp now);
	static int64_t numCreated() { return s_numCreated.get(); }
private:
	const TimerCallback m_callback;
	Timestamp m_expiration;
	const double m_interval;
	const bool is_repeat;
	const int64_t m_sequence;

	static AtomicInt64 s_numCreated;
};


}








}


#include "dc/net/timer.h"

namespace dc
{
namespace net
{
AtomicInt64 Timer::s_numCreated;

void Timer::restart(Timestamp now)
{
	if(is_repeat)
	{
		m_expiration = addTime(now, m_interval);
	}
	else
	{
		m_expiration = Timestamp::invalid();
	}
}

}

}


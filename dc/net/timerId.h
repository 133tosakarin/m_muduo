#ifndef DC_NET_TIMERID_H
#define DC_NET_TIMERID_H


namespace dc
{
namespace net
{
class Timer;
class TimerId
{
public:
	TimerId()
		: m_timer(nullptr), m_sequence(0)
	{ }
	TimerId(Timer* timer, int64_t seq)
	  : m_timer(timer),
	  	m_sequence(seq)
	{ }

	friend class TimerQueue;
private:
	Timer* m_timer;
	int64_t m_sequence;

};

}
}



#endif

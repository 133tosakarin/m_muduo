#ifndef DC_BASE_COUNTDOWNLATCH_H
#define DC_BASE_COUNTDOWNLATCH_H
#include "mutex.h"
#include "condition.h"

namespace dc
{
class CountDownLatch : noncopyable
{
public:
	explicit CountDownLatch( int count );
	
	void wait();

	void countDown();

	int getCount() const;
private:
	mutable MutexLock m_mutex;
	Condition m_cond GUARDED_BY(m_mutex);
	int m_count GUARDED_BY(m_mutex);
};

}

#endif

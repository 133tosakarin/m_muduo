#include"condition.h"
#include<errno.h>

bool dc::Condition::waitForSeconds( double seconds )
{
	struct timespec abstime;
	
	// FIXME: use CLOCK_MONOTONIC or CLOCK_MONONTONIC_RAW t oprevent time rewind
	clock_gettime(CLOCK_REALTIME, &abstime);
	const int64_t kNanoSecondsPerSecond = 1e9;
	int64_t nanoseconds = static_cast<int64_t>( seconds * kNanoSecondsPerSecond);
	abstime.tv_sec +=static_cast<time_t>( (abstime.tv_nsec + nanoseconds ) / kNanoSecondsPerSecond);
	abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds ) % kNanoSecondsPerSecond );
	MutexLock::UnassignGuard ug(m_mutex);
	return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.getPthreadMutex(), &abstime);

}



#ifndef DC_BASE_CURRENTTHREAD_H
#define DC_BASE_CURRENTTHREAD_H

#include "types.h"

namespace dc
{
namespace CurrentThread
{
extern thread_local int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cachedTid();

inline int tid()
{
	if(__builtin_expect(t_cachedTid == 0, 0 ) )
	{
		cachedTid();
	}
	return t_cachedTid;
}

inline const char* tidString()
{
	return t_tidString;
}

inline int tidStringLength()
{
	return t_tidStringLength;
}

inline const char* name()
{
	return t_threadName;
}

bool isMainThread();
void sleepUsec(int64_t usec);

std::string stackTrace(bool demangle);

}
}

#endif

#ifndef DC_BASE_THREAD_H
#define DC_BASE_THREAD_H
#include "atomic.h"
#include "countDownLatch.h"
#include "types.h"
#include <functional>
#include <memory>
#include <pthread.h>

namespace dc
{
class Thread : noncopyable
{
public:
	using ThreadFunc = std::function<void()>;
	explicit Thread( ThreadFunc, const std::string& name = string());
	//FixME: make it movable in c++11
	~Thread();

	void start();
	int join(); //return pthread_join()

	bool started() const { return is_started;}
	pid_t tid() const { return m_pid; }
	const std::string& name() const { return m_name; }
	static int numCreated() { return m_numCreated.get(); } 
private:
	void setDefaultName();

	bool is_started;
	bool is_joined;
	pthread_t m_threadId;
	pid_t m_pid;
	ThreadFunc m_func;
	
	std::string m_name;
	CountDownLatch m_latch;
	static AtomicInt32 m_numCreated;

};
}

#endif

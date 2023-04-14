
#include	<stdlib.h>
#include <cstdio>
#include "thread.h"
#include "currentThread.h"
#include "threadLocalSingleton.h"
#include "singleton.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

class Test
{
public:
	Test()
	{
		printf("tid = %d, constructing %p\n",
				dc::CurrentThread::tid(), this);
	}
	~Test()
	{
		printf("tid = %d, destructing %p %s\n",
				dc::CurrentThread::tid(),
				this,
				name_.c_str());
	}
	const dc::string& name() const
	{
		return name_;
	}
	void setName(const dc::string& name)
	{
		name_ = std::move(name);
	}
private:
	dc::string name_;
};

void threadFunc(const char* changeInfo)
{
	printf("tid = %d, %p name = %s\n",
			dc::CurrentThread::tid(),
			&dc::ThreadLocalSingleton<Test>::instance(),
			dc::ThreadLocalSingleton<Test>::instance().name().c_str());
	dc::ThreadLocalSingleton<Test>::instance().setName(changeInfo);
	printf("tid = %d, %p name = %s\n",
			dc::CurrentThread::tid(),
			&dc::ThreadLocalSingleton<Test>::instance(),
			dc::ThreadLocalSingleton<Test>::instance().name().c_str());
}
	int
main ( int argc, char *argv[] )
{
	dc::ThreadLocalSingleton<Test>::instance().setName("main one");
	dc::Thread t1(std::bind(threadFunc, "thread1"));
	dc::Thread t2(std::bind(threadFunc, "thread2"));
	t1.start();
	t2.start();
	t1.join();
	printf("tid = %d, %p name = %s\n",
			dc::CurrentThread::tid(),
			&dc::ThreadLocalSingleton<Test>::instance(),
			dc::ThreadLocalSingleton<Test>::instance().name().c_str());
	t2.join();
	pthread_exit(0);
}				/* ----------  end of function main  ---------- */

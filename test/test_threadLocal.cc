
#include	<stdlib.h>
#include <cstdio>
#include <string>
#include "threadLocal.h"
#include "thread.h"
#include "currentThread.h"
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 *
 */
class Test;
dc::ThreadLocal<Test> testobj1;
dc::ThreadLocal<Test> testobj2;

class Test
{
public:
	Test()
	{
		printf("constrctor testobj1 = %p\n", &testobj1);
		printf("constrctor testobj2 = %p\n", &testobj2);
	}
	void setName(const std::string& name)
	{
		name_ = name;
	}

	std::string name()
	{
		return name_;
	}
private:
	std::string name_;
};



void print()
{
	printf("tid = %d, name = %s, thread addr = %p \n", dc::CurrentThread::tid(), testobj1.value().name().c_str(), &testobj1.value());
	printf("tid = %d, name = %s, thread addr = %p \n", dc::CurrentThread::tid(), testobj2.value().name().c_str(), &testobj2.value());
}

void threadFunc()
{
	print();
	testobj1.value().setName("channged 1");
	testobj2.value().setName("change 2");
	print();
}

	int
main ( int argc, char *argv[] )
{
	testobj1.value().setName("main one");
	print();
	dc::Thread t1(threadFunc);
	t1.start();
	t1.join();
	testobj2.value().setName("main tow");
	print();

	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

/*
 * =====================================================================================
 *
 *       Filename:  test_exception.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/06/2023 06:51:30 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "exception.h"
#include "currentThread.h"
#include <functional>
#include <vector>
#include	<stdlib.h>
#include <cstdio>

class Bar
{
public:
	void test(std::vector<std::string> names = {} )
	{
		printf("Stack:\n%s\n", dc::CurrentThread::stackTrace(true).c_str());
		[] {
			printf("Stack inside lambda:\n%s\n", dc::CurrentThread::stackTrace(true).c_str());
		}();

		std::function<void()> func([]
				{
					printf("Stack inside std::function:\n%s\n", dc::CurrentThread::stackTrace(true).c_str());	
				});
		func();
		func = std::bind(&Bar::callback, this);
		func();
		throw dc::Exception("oops");
	}
private:
	void callback()
	{
		printf("Stack inside std::bind:\n%s\n", dc::CurrentThread::stackTrace(true).c_str());
	}
};
void foo()
{
	Bar b;
	b.test();
}

	int
main ( int argc, char *argv[] )
{	
	try
	{
		foo();
	
	}
	catch(const dc::Exception& ex)
	{
		printf("reason: %s\n", ex.what());
		printf("stack trace:\n%s\n", ex.stackTrace());
	}
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

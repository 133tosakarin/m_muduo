
#include	<stdlib.h>
#include <string>
#include <cstdio>
#include "thread.h"
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
	void setName(const std::string& name_)
	{
		name = std::move(name_);
	}

	std::string getName()
	{
		return name;
	}
private:
	std::string name;
};

void func()
{
	printf("name = %s\n", dc::detail::Singleton<Test>::instance().getName().c_str());
}
	int
main ( int argc, char *argv[] )
{
	dc::detail::Singleton<Test>::instance().setName("only one");
	printf("main func name = %s\n",dc::detail::Singleton<Test>::instance().getName().c_str());

	dc::Thread thr(func);
	thr.start();
	thr.join();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

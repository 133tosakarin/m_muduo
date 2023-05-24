
#include	<stdlib.h>
#include <assert.h>
#include <cstdio>
#include <unistd.h>

#include "dc/net/timerId.h"
#include "dc/net/eventLoop.h"
#include "dc/base/thread.h"

using namespace dc;
using namespace dc::net;
EventLoop* g_loop;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

void callback()
{
	printf("callback() : pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	//EventLoop anotherLoop;
}

void threadfunc()
{
	printf("threadfunc: pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
	EventLoop loop;
	assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

	loop.runAfter(1.0, callback);
	loop.loop();

}

	int
main ( int argc, char *argv[] )
{
	printf("main: pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
	EventLoop loop;
	assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
	g_loop = &loop;	
	Thread thread(threadfunc);
	thread.start();
	loop.runAfter(2.0, callback);
	loop.loop();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

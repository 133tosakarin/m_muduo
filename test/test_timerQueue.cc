#include "dc/net/timerQueue.h"
#include "dc/net/eventLoop.h"
#include "dc/net/eventLoopThreadPool.h"
#include "dc/net/eventLoopThread.h"
#include "dc/base/logging.h"
#include <cstdio>
using namespace dc;
using namespace dc::net;
EventLoop* g_loop;
int cnt;
void printId()
{
    printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    printf("now %s\n", Timestamp::now().toString().c_str());
}

void print( const char* msg)
{
    printf("msg %s %s\n", Timestamp::now().toString().c_str(), msg );
    if( ++cnt == 20 )
    {
        g_loop->quit();
    }
}

void cancel(TimerId timer)
{
    g_loop->cancel(timer);
    printf("canceled at %s\n", Timestamp::now().toString().c_str());
}
int main()
{
    //g_logLevel = Logger::TRACE;
    printId();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;
        print("main");
        loop.runAfter(1, std::bind(print, "once1"));
        loop.runAfter(1.5, std::bind(print, "once1.5"));
        loop.runAfter(2.5, std::bind(print, "once3.5"));
        TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
        loop.runAfter(4.2, std::bind(cancel, t45));
        loop.runAfter(4.8, std::bind(cancel, t45));
        loop.runEvery(2, std::bind(print, "every2"));
        TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
        loop.runAfter(9.001, std::bind(cancel, t3));
        loop.loop();
        print("main loop exits");
    }
    sleep(1);
    {
        EventLoopThread loopThread;
        EventLoop* loop = loopThread.startLoop();
        loop->runAfter(2, printId);
        sleep(3);
        print("thread loop exits");
    }
    return 0;
}
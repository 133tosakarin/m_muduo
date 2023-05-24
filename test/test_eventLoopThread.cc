#include "dc/net/eventLoopThread.h"
#include "dc/base/currentThread.h"
#include "dc/net/eventLoop.h"
#include <cstdio>
#include <unistd.h>
#include <chrono>
using namespace dc;
using namespace dc::net;

void print(EventLoop* p = nullptr)
{
    printf("print: pid = %d, tid = %d, loop = %p\n", getpid(), CurrentThread::tid(), p);
}
int main()
{
    print();
    {
        EventLoopThread thr1; //never start
    }

    {
        EventLoopThread thr2;
        auto loop = thr2.startLoop();
        loop->runInLoop([loop]{ print(loop); loop->quit(); });
        CurrentThread::sleepUsec(500 * 100000);
    }

    {
        //CurrentThread::sleepUsec(500 * 1000);
        EventLoopThread thr3;
        auto loop = thr3.startLoop();
        loop->runInLoop([loop]{ print(loop); loop->quit(); });
        //CurrentThread::sleepUsec(500 * 1000);
    }
    return 0;
}

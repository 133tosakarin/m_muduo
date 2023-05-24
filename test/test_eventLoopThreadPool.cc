#include "dc/net/eventLoopThread.h"
#include "dc/net/eventLoop.h"
#include "dc/net/eventLoopThreadPool.h"
#include "dc/net/timerId.h"
#include <cstdio>

using namespace dc;
using namespace dc::net;

void print(EventLoop* loop = nullptr)
{
    printf("main: pid = %d, tid = %d, loop = %p\n", getpid(), CurrentThread::tid(), loop);
}

void init(EventLoop* loop = nullptr)
{
    printf("init: pid = %d, tid = %d, loop = %p\n", getpid(), CurrentThread::tid(), loop );
}
int main()
{
    print();
    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));
    {
        printf("Single Thread :%p \n", &loop);
        EventLoopThreadPool pool1(&loop, "Single");
        pool1.setThreadNum(0);
        pool1.start(init);
        auto nextLoop = pool1.getNextLoop();
        assert(nextLoop == &loop);
        assert(pool1.getNextLoop() == &loop);
        assert(pool1.getNextLoop() == &loop);

    }
    {
        printf("Another Thread :%p \n", &loop);
        EventLoopThreadPool pool2(&loop, "Another");
        pool2.setThreadNum(1);
        pool2.start(std::bind(init, &loop));
        auto nextLoop = pool2.getNextLoop();
        
        nextLoop->runInLoop(std::bind(print, nextLoop));
        assert(nextLoop != &loop );
        assert(nextLoop == pool2.getNextLoop());
    }

    {
        printf("Three Thread :%p\n", &loop);
        EventLoopThreadPool pool2(&loop, "Three");
        pool2.setThreadNum(3);
        pool2.start(std::bind(init,&loop));

        auto nextLoop = pool2.getNextLoop();
        nextLoop->runInLoop([nextLoop]{print(nextLoop);});
        assert(nextLoop != &loop );
        assert(pool2.getNextLoop() != nextLoop);
        assert(pool2.getNextLoop() != nextLoop );
        assert(pool2.getNextLoop() == nextLoop);
        auto vec = pool2.getAllLoops();
        for( auto loop : vec)
        {
            loop->runInLoop([loop]{print(loop);});
        }
    }
    loop.loop();
    return 0;
}
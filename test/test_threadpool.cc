#include "../dc/base/threadpool.h"
#include "../dc/base/logging.h"
#include "../dc/base/currentThread.h"
void print()
{
    printf("tid = %d, name = %s\n", dc::CurrentThread::tid(), dc::CurrentThread::name());
}

void printstr(const std::string& str)
{
    LOG_INFO << str;
    usleep(100 * 1000);
}

void test(int maxSize)
{
    LOG_WARN << "Test ThreadPool with mas queue size = " << maxSize;
    dc::ThreadPool pool("mainThreadpool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    LOG_WARN << "Adding tasks";
    pool.run(print);
    pool.run(print);
    for( int i = 0; i < 100; ++i )
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d, thread_name = %s", i, dc::CurrentThread::name());
        pool.run(std::bind(printstr, std::string(buf)));
    }
    LOG_WARN << "Done";

    dc::CountDownLatch  latch(1);
    pool.run(std::bind(&dc::CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

void longTask(int num)
{
    LOG_INFO << "longTask" << num;
    dc::CurrentThread::sleepUsec(3000000);
}

void test2()
{
    LOG_WARN << "Test ThreadPool by stoping early.";
    dc::ThreadPool pool("ThreadPool");
    pool.setMaxQueueSize(5);
    pool.start(3);

    dc::Thread thread1([&pool]()
    {
        for( int i = 0; i < 20; ++i )
        {
            pool.run(std::bind(longTask, i));
        }
    }, "thread1");
    thread1.start();
    dc::CurrentThread::sleepUsec(5000000);
    LOG_WARN << "stop pool";
    pool.stop();
    thread1.join();
    pool.run(print);
    LOG_WARN << "test2 Done";
}
int main()
{
    test(0);
    test(1);
    test(5);
    test(10);
    test(50);
    test2();
    return 0;
}
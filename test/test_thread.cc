#include "../dc/base/thread.h"
#include "../dc/base/threadpool.h"
#include "../dc/base/currentThread.h"
#include <pthread.h>
#include <unistd.h>
#include "../dc/base/logging.h"
#include <fcntl.h>
void threadFunc()
{
    printf("pid = %d, tid = %d\n", ::getpid(), dc::CurrentThread::tid());
}

class Foo
{
public:
    Foo() : _x(123.1) {}
    Foo(double x) : _x(x) {}
    void memberfunc()
    {
        printf("foo mem func test: pid = %d, tid = %d, foo._x = %f, name = %s\n", ::getpid(), dc::CurrentThread::tid(), _x, dc::CurrentThread::name());
    }

    void memberfunc2(std::string& str)
    {
        printf("foo mem func2 test: pid = %d, tid = %d, str = %s, thread name = %s\n", ::getpid(), dc::CurrentThread::tid(), str.c_str(), dc::CurrentThread::name());
    }
private:
    double _x;

};
int main()
{
    LOG_INFO << "test_thread start";
    printf("pid = %d, tid = %d\n", ::getpid(), dc::CurrentThread::tid());

    dc::Thread thread1(threadFunc);
    thread1.start();

    printf("tid = %d\n", dc::CurrentThread::tid());
    thread1.join();
    Foo foo(1212451);
    dc::Thread t2(std::bind(&Foo::memberfunc, &foo));

    t2.start();

    LOG_INFO << "test t2";

    t2.join();
    dc::Thread t3(std::bind(&Foo::memberfunc2, std::ref(foo), std::string("hello world")), "dengchao");
    t3.start();

    LOG_INFO << "test t3";
    t3.join();

    LOG_INFO << "test_thread end";
  	printf("created %d threads\n", dc::Thread::numCreated());
	return 0;
}

#include "../dc/base/mutex.h"
#include "../dc/base/countDownLatch.h"
#include "../dc/base/thread.h"
#include "../dc/base/timeStamp.h"

#include <vector>
#include <cstdio>

using namespace std;
dc::MutexLock g_mutex;
vector<int> g_vec;

const int kCount = 10*1000*1000;

void threadFunc()
{
    for( int i = 0; i < kCount; ++i )
    {
        dc::MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int foo() __attribute__ ((noinline));

int g_count = 0;
int foo()
{
    dc::MutexLockGuard lock(g_mutex);
    if( !g_mutex.isLockedByThisThread())
    {
        printf("FATL\n");
        return -1;
    }
    ++g_count;
    return 0;
}

int main()
{
    printf("sizeof pthread_mutex_t: %zd\n", sizeof(pthread_mutex_t));
    printf("sizeof Mutex: %zd\n", sizeof(dc::MutexLock));
    printf("sizeof pthread_cond_t: %zd\n", sizeof(pthread_cond_t));
    printf("sizeof Condition %zd\n", sizeof(dc::Condition));
    MCHECK(foo());
    if(g_count!=1)
    {
        printf("MCHECK calls twice.\n");
        abort();
    }   

    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);

    dc::Timestamp start(dc::Timestamp::now());
    for (int i = 0; i < kCount; ++i)
    {
        g_vec.push_back(i);
    }
    printf("single thread without lock %f\n", dc::timeDifference(dc::Timestamp::now(), start));
    start = dc::Timestamp::now();
    threadFunc();
    printf("single thread with lock %f\n", dc::timeDifference(dc::Timestamp::now(), start));
    for( int ntrheads = 1; ntrheads < kMaxThreads;++ntrheads)
    {
        std::vector<std::unique_ptr<dc::Thread>> threads;
        g_vec.clear();
        start = dc::Timestamp::now();
        for( int i = 0; i < ntrheads; ++i )
        {
            threads.emplace_back(new dc::Thread(&threadFunc));
            threads.back()->start();
        }   
        for( int i = 0; i < ntrheads; ++i)
        {
            threads[i]->join();
        }
        printf("%d threads with lock %f\n", ntrheads, dc::timeDifference(dc::Timestamp::now(), start));
    }
}


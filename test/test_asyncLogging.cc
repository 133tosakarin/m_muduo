#include "../dc/base/asyncLogging.h"
#include "../dc/base/logging.h"
#include "../dc/base/timeStamp.h"
#include "../dc/base/exception.h"
#include <cstdio>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>


off_t kRollSize = 500 * 1000 * 1000;

dc::AsyncLogging* g_logger = nullptr;
//std::shared_ptr<dc::AsyncLogging> g_logger;
void asyncOutput(const char* msg, int len)
{
    g_logger->append(msg, len);
}

void bench(bool longLog)
{
    dc::Logger::setOutput(asyncOutput);

    int cnt = 0;
    const int KBatch = 1000;
    dc::string empty = " ";
    dc::string longStr(3000, 'X');
    longStr +=" ";
    try{
        for( int t = 0; t < 30; ++t)
        {
            dc::Timestamp start = dc::Timestamp::now();
            for( int i = 0; i < KBatch; ++i )
            {
                LOG_INFO << "hello 01234556789" << "abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty) << cnt;
                ++cnt;
            }
            dc::Timestamp end = dc::Timestamp::now();
            printf("%f\n", dc::timeDifference(end, start) * 1000000/KBatch);
            struct timespec ts = {0, 500*1000*1000};
            nanosleep(&ts, nullptr);
        }
    } catch(dc::Exception& ex)
    {
        printf("%s\n%s", ex.what(), ex.stackTrace());
    }
}
int main(int argc, char**argv)
{
    {
        // set max virtual memory to 2GB
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = {2*kOneGB, 2*kOneGB};
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());

    char name[256] = {'\0'};
    strncpy(name, argv[0], sizeof(name) - 1);
    //std::shared_ptr<dc::AsyncLogging>log(new dc::AsyncLogging(::basename(name), kRollSize));
    dc::AsyncLogging log(::basename(name), kRollSize);
    log.start();
    g_logger = &log;
    bool longLog = argc > 1;
    bench(longLog);
    return 0;
}

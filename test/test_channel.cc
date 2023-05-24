#include "logging.h"
#include "../dc/net/channel.h"
#include "../dc/net/eventLoop.h"
#include "../dc/net/timerId.h"
#include <functional>
#include <map>
#include <cstdio>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace dc;
using namespace dc::net;

void print(const char* msg)
{
    static std::map<const char*,  Timestamp> lasts;
    Timestamp& last = lasts[msg];
    Timestamp now = Timestamp::now();

    printf("%s tid %d %s delay %f\n", now.toString().c_str(), CurrentThread::tid(), msg, timeDifference(now, last));
    last = now;
}

namespace dc
{
namespace net
{
namespace detail
{
    int createTimerfd();
    void readTimerfd(int timerfd, Timestamp now);
}
}
}

class PeriodicTimer
{
public:
    PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb )
        : m_loop(loop),
          m_timerfd(dc::net::detail::createTimerfd()),
          m_timerfdChannel(loop, m_timerfd),
          m_interval(interval),
          m_cb(cb)
    {
        LOG_TRACE << "PeriodTimer fd = " << m_timerfd;
        m_timerfdChannel.setReadCallback(std::bind(&PeriodicTimer::handleRead, this));
        m_timerfdChannel.enableReading();
    }

    void start()
    {
        struct itimerspec spec;
        memZero(&spec, sizeof spec);
        spec.it_interval = toTimeSpec(m_interval);
        spec.it_value = spec.it_interval;

        int ret = ::timerfd_settime(m_timerfd, 0, &spec, nullptr);
        if( ret )
        {
            LOG_SYSERR << "timerfd_settime()";
        }
    }
    ~PeriodicTimer()
    {
        m_timerfdChannel.disableAll();
        m_timerfdChannel.remove();
        ::close(m_timerfd);
    }
private:

    void handleRead()
    {
        m_loop->assertInLoopThread();
        dc::net::detail::readTimerfd(m_timerfd, Timestamp::now());
        if( m_cb )
            m_cb();
    }

    static struct timespec toTimeSpec(double seconds)
    {
        struct timespec ts;
        memZero(&ts, sizeof ts);
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;

        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if(nanoseconds < kMinInterval )
        {
            nanoseconds = kMinInterval;
        }
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }
    EventLoop* m_loop;
    const int m_timerfd;
    Channel m_timerfdChannel;
    const double m_interval;
    TimerCallback m_cb;
};
int main()
{
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid()
                         << " Try adjusting the wall clock, see what happens";
    g_logLevel = dc::Logger::TRACE;
    EventLoop loop;
    PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
    timer.start();
    loop.runEvery(1, std::bind(print, "EventLoop::runEvery"));
    loop.loop();
}
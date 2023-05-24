#include "dc/net/eventLoop.h"
#include "dc/base/logging.h"
#include "dc/net/poller.h"
#include "dc/net/socketsOps.h"
#include "dc/net/timerQueue.h"
#include "dc/net/channel.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace
{
thread_local dc::net::EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if( evtfd < 0 )
	{
		LOG_SYSERR << "failed in eventfd";
		abort();
	}

	return evtfd;
}
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);

	}
};

#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}// namespace 


namespace dc
{
namespace net
{

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

EventLoop::EventLoop()
	: is_looping( false ),
	  is_quit( false ),
	  is_eventHandling( false ),
	  is_callingPendingFunctors( false ),
	  m_iteration( 0 ),
	  m_threadId( CurrentThread::tid() ),
	  m_poller(Poller::newDefaultPoller(this)),
	  m_timerQueue(new TimerQueue(this)),
	  m_wakeupFd(createEventfd()),
	  m_wakeupChannel(new Channel(this, m_wakeupFd)),
	  m_currentActiveChannel(nullptr)
{
	LOG_DEBUG << "EventLoop created " << this << "in thread " << m_threadId;
	if( t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " 
		<< t_loopInThisThread << " exists in this thread " << m_threadId;
	}
	else
	{
		t_loopInThisThread = this;
	}

	m_wakeupChannel->setReadCallback( std::bind(&EventLoop::handleRead, this) );
	m_wakeupChannel->enableReading();

}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << m_threadId
			  << " destructs in thread " << CurrentThread::tid();
	m_wakeupChannel->disableAll();
	m_wakeupChannel->remove();
	::close(m_wakeupFd);
	t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
	//LOG_INFO << "EventLoop " << this << "first into ";
	assert(!is_looping);
	assertInLoopThread();
	is_looping = true;
	is_quit = false;
	LOG_TRACE << "EventLoop " << this << " start looping";
	while(!is_quit)
	{
		m_activeChannels.clear();
		m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
		++m_iteration;
		if( Logger::logLevel() <= Logger::TRACE)
		{
			printActiveChannels();
		}
		is_eventHandling = true;
		for(Channel* channel : m_activeChannels)
		{
			m_currentActiveChannel = channel;
			m_currentActiveChannel->handleEvent(m_pollReturnTime);
		}

		m_currentActiveChannel = nullptr;
		is_eventHandling = false;
		doPendingFunctors();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
	is_looping = false;
}

void EventLoop::quit()
{
	is_quit = true;
	if( !isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::runInLoop(Functor cb)
{
	if(isInLoopThread())
	{
		cb();

	}
	else
	{
		queueInLoop(std::move(cb));
	}

}

void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexLockGuard lock(m_mutex);
		m_pendingFunctors.push_back(std::move(cb));
	}
	if(!isInLoopThread() || is_callingPendingFunctors)
	{
		wakeup();
	}
}

size_t EventLoop::queueSize() const
{
	MutexLockGuard lock(m_mutex);
	return m_pendingFunctors.size();
}
TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
	return m_timerQueue->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
	//LOG_INFO << "add a timer delay = " << delay;
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return m_timerQueue->addTimer(std::move(cb), time, interval); 
}

void EventLoop::cancel(TimerId timerId)
{
	return m_timerQueue->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	m_poller->updateChannel(channel);
}


void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this );
	assertInLoopThread();
	if( is_eventHandling )
	{
		assert( (m_currentActiveChannel == channel ) || (std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) == m_activeChannels.end() ));
	}
	m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this );
	assertInLoopThread();
	return m_poller->hasChannel(channel);
}
void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this << " was created in threadId = " << m_threadId << ", current thread id =  " << CurrentThread::tid();
}


void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = sockets::write(m_wakeupFd, &one, sizeof one);
	if( n != sizeof one )
	{
		LOG_ERROR << "EventLoop::wakeup() writes" << n << " bytes instead of 8";
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(m_wakeupFd, &one, sizeof one);
	if( n != sizeof one )
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n  << " bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	is_callingPendingFunctors = true;
	{
		MutexLockGuard lock(m_mutex);
		functors.swap(m_pendingFunctors);
	}
	for( const Functor& functor : functors)
	{
		functor();
	}
	is_callingPendingFunctors = false;
}



void EventLoop::printActiveChannels() const
{
	for(const Channel* channel : m_activeChannels )
	{
		LOG_TRACE << "{" << channel->reventsToString() << "} ";
	}
}
}//net

}//dc

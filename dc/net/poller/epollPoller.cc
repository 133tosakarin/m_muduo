#include "dc/net/poller/epollPoller.h"
#include "dc/base/logging.h"
#include "dc/net/channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace dc;
using namespace dc::net;

static_assert(EPOLLIN == POLLIN,  	"epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,  	"epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,  	"epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  	"epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,  	"epoll uses same flag values as poll");
static_assert(EPOLLHUP== POLLHUP,  	"epoll uses same flag values as poll");

namespace 
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}
EpollPoller::EpollPoller(EventLoop* loop) : Poller(loop), m_epollfd(::epoll_create1(EPOLL_CLOEXEC)), m_events(kInitEventListSize)
{
	LOG_TRACE << "epollfd = " << m_epollfd;
	if( m_epollfd < 0 )
	{
		LOG_SYSFATAL << "EpollPoller::EpollPoller";
	}
}

EpollPoller::~EpollPoller()
{
	::close( m_epollfd );
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
	LOG_TRACE << "fd total count "  << m_channels.size();
	int numEvents = ::epoll_wait(m_epollfd, m_events.data(), static_cast<int>(m_events.size()), timeoutMs);
	int saveErrno = errno;
	Timestamp now(Timestamp::now());
	if( numEvents > 0 )
	{
		LOG_TRACE << numEvents << " events happened";
		fillActiveChannels(numEvents, activeChannels);
		if( implicit_cast<size_t>(numEvents) == m_events.size() )
		{
			m_events.resize(m_events.size() * 2);
		}
	}
	else if( numEvents == 0 )
	{
		LOG_TRACE << "nothing happend";
	
	}
	else
	{
		if( saveErrno != EINTR )
		{
			errno = saveErrno;
			LOG_SYSERR << "EpollPoller::poll()";
		}
	}
	return now;
}
void EpollPoller::fillActiveChannels( int numEvents, ChannelList* activeChannels) const
{
	assert( implicit_cast<size_t>(numEvents) <= m_events.size());
	for( int i = 0; i < numEvents; ++i )
	{
		Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);
		LOG_TRACE << "fillActiveChannel, channel->fd() = " << channel->fd();
	#ifndef NDEBUG
		int fd = channel->fd();
		auto it = m_channels.find(fd);
		assert(it != m_channels.end());
		assert(it->second == channel);
	#endif
		channel->set_revents(m_events[i].events);
		activeChannels->push_back(channel);

	}

}
void EpollPoller::updateChannel( Channel* channel )
{
	assertInLoopThread();
	const int index = channel->index();
	LOG_TRACE << "fd = "  << channel->fd() 
			<< " events = " << channel->events() << " index = " << index;
	if( index == kNew || index == kDeleted)
	{
		// anew one , add with EPOLL_CTL_ADD
		int fd = channel->fd();
		if (index == kNew)
		{
			assert(m_channels.find(fd) == m_channels.end());
			m_channels[fd] = channel;
		}
		else
		{
			assert(m_channels.find(fd) != m_channels.end());
			assert(m_channels[fd] == channel);
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		// update existing one with EPOLL_CTL_MOD/DEL
		int fd = channel->fd();
		//void(fd);
		assert( m_channels.find(fd) != m_channels.end());
		assert( m_channels[fd] == channel );
		assert(index == kAdded);
		if( channel->isNoneEvent() )
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EpollPoller::removeChannel(Channel* channel)
{
	assertInLoopThread();
	int fd = channel->fd();
	LOG_TRACE << "fd = " << fd;
	assert(m_channels.find(fd) != m_channels.end());
	assert(m_channels[fd] == channel);
	assert(channel->isNoneEvent());
	int index = channel->index();
	assert(index == kAdded || index == kDeleted);
	size_t n = m_channels.erase(fd);
	(void)n;
	assert(n == 1 );
	if( index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}

void EpollPoller::update(int operation, Channel* channel )
{
	struct epoll_event event;
	memZero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
			 << " fd = " << fd << " event = { "  << channel->eventsToString() << " } ";
	if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
	{


		
		if( operation == EPOLL_CTL_DEL )
		{
			LOG_SYSERR << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
		}
		else
		{
			LOG_SYSFATAL << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;

		}
	}
}
const char* EpollPoller::operationToString(int op)
{
	switch (op)
	{
		case EPOLL_CTL_ADD:
			return "ADD";
		case EPOLL_CTL_MOD:
			return "MOD";
		case EPOLL_CTL_DEL:
			return "DEL";
		default:
			assert(false && "ERROR op");
			return "Unknown Operation";
	}
}








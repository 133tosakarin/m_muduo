#include "dc/net/poller/pollPoller.h"
#include "dc/base/logging.h"
#include "dc/net/channel.h"
#include "dc/base/types.h"
#include <poll.h>
#include <assert.h>
#include <errno.h>

using namespace dc;
using namespace dc::net;

PollPoller::PollPoller(EventLoop* loop) : Poller(loop)
{


}
PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll( int timeoutMs, ChannelList* activeChannels )
{
	int numEvents = ::poll(m_pollfds.data(), m_pollfds.size(), timeoutMs );
	int saveErrno = errno;
	Timestamp now(Timestamp::now());
	if( numEvents > 0 )
	{
		LOG_TRACE << numEvents << " events happend";
		fillActiveChannels(numEvents, activeChannels);
	}
	else if( numEvents == 0 )
	{
		LOG_TRACE << " nothing happened";	
	}
	else
	{
		if( saveErrno != EINTR )
		{
			errno = saveErrno;
			LOG_SYSERR << "PollPoller::poll()";
		}
	}
	return now;	

}
void PollPoller::updateChannel(Channel* channel)
{
	assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
	if( channel->index() < 0 )
	{
		assert(m_channels.find(channel->fd()) == m_channels.end());
		struct pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		m_pollfds.push_back(pfd);
		int idx = m_pollfds.size() - 1;
		channel->set_index( idx );
		m_channels[pfd.fd] = channel;
	}
	else
	{
		assert(m_channels.find(channel->fd()) != m_channels.end());
		assert(m_channels[channel->fd()] == channel);
		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
		struct pollfd& pfd = m_pollfds[idx];
		assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
		pfd.fd = channel->fd();
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent())
		{
			pfd.fd = -channel->fd() - 1;
		}
	}
}

void PollPoller::removeChannel( Channel* channel )
{
	assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd();
	assert(m_channels.find(channel->fd()) != m_channels.end());
	assert(m_channels[channel->fd()] == channel);
	assert(channel->isNoneEvent());
	int idx = channel->index();
	assert( 0 <= idx && idx < static_cast<int>(m_pollfds.size()));
	const struct pollfd pfd = m_pollfds[idx];(void)pfd;
	assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
	size_t n = m_channels.erase(channel->fd());
	assert(n == 1); (void)n;
	if( implicit_cast<size_t>(idx) == m_pollfds.size() - 1 )
	{
		m_pollfds.pop_back();
	}
	else
	{
		int channelAtEnd = m_pollfds.back().fd;
		iter_swap(m_pollfds.begin() + idx, m_pollfds.end() - 1);
		if( channelAtEnd < 0 )
		{
			channelAtEnd = -channelAtEnd - 1;
		}
		m_channels[channelAtEnd]->set_index(idx);
		m_pollfds.pop_back();
	}
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels ) const
{
	for( auto pfd = m_pollfds.begin(); pfd != m_pollfds.end() && numEvents > 0; pfd++ )
	{

		if( pfd->revents > 0 )
		{
			--numEvents;
			auto ch = m_channels.find(pfd->fd);
			assert( ch != m_channels.end() );
			Channel* channel = ch->second;
			assert(channel->fd() == pfd->fd);
			channel->set_revents(pfd->revents);
			activeChannels->push_back(channel);
		}
	}

}










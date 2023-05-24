#include "dc/net/poller.h"
#include "dc/net/channel.h"


namespace dc
{
namespace net
{

Poller::Poller(EventLoop* loop) : m_ownerLoop(loop)
{

}

Poller::~Poller() = default;
bool Poller::hasChannel(Channel* channel ) const
{

	assertInLoopThread();
	auto it = m_channels.find(channel->fd());
	return it != m_channels.end() && it->second == channel;
}



} //namespace net
}//namespace dc

#include "dc/net/poller.h"
#include "dc/net/poller/pollPoller.h"
#include "dc/net/poller/epollPoller.h"

#include <cstdlib>

using namespace dc::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
	if( ::getenv("DC_USE_POLL"))
	{
		return new PollPoller(loop);
	}
	else
	{
		return new EpollPoller(loop);
	}
}

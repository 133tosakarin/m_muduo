#ifndef DC_NET_POLLER_EPOLLPOLLER_H
#define DC_NET_POLLER_EPOLLPOLLER_H

#include "dc/net/poller.h"
#include <vector>

struct epoll_event;

namespace dc
{
namespace net
{
class EpollPoller : public Poller
{
public:

	EpollPoller(EventLoop* loop);
	~EpollPoller() override;

	Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
	void updateChannel( Channel* channel ) override;
	void removeChannel( Channel* channel ) override;

private:
	static const int kInitEventListSize = 16;
	static const char* operationToString( int op );
	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

	void update(int operation, Channel* channel );
	using EventList = std::vector<struct epoll_event>;
	int m_epollfd;
	EventList m_events;

};

}/// namespace net

}// namespace dc
#endif

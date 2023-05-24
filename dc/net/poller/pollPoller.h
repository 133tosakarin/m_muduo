#ifndef DC_NET_POLLER_POLLERPOLLER_H
#define DC_NET_POLLER_POLLERPOLLER_H
#include "dc/net/poller.h"

#include <vector>


struct pollfd;

namespace dc
{
namespace net
{
class PollPoller : public Poller
{
public:
	PollPoller(EventLoop * loop);
	~PollPoller() override;
	Timestamp poll(int timeoutMs, ChannelList* activeChannels ) override;
	void updateChannel(Channel* channel ) override;
	void removeChannel(Channel* channel ) override;
private:
	void fillActiveChannels(int numEvents, ChannelList* activeChannels ) const;
	using PollerFdList = std::vector<pollfd>;
	PollerFdList m_pollfds;

};

}///namespace net

}// namespace dc
#endif

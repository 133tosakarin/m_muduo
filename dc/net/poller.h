#ifndef DC_NET_POLLER_H
#define DC_NET_POLLER_H

#include "dc/base/timeStamp.h"
#include "dc/net/eventLoop.h"

#include <map>
#include <vector>

namespace dc
{
namespace net
{

class Poller : noncopyable
{
public:
	using ChannelList = std::vector<Channel* >;
	Poller(EventLoop* loop);
	virtual ~Poller();

	///Polls the I/O events.
	/// Must be called in the loop thread
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels ) = 0;
	virtual void updateChannel(Channel* channel ) = 0;

	virtual void removeChannel(Channel* channel) = 0;

	virtual bool hasChannel(Channel* channel ) const;

	static Poller* newDefaultPoller( EventLoop* loop );

	void assertInLoopThread() const
	{
		m_ownerLoop->assertInLoopThread();
	}
protected:
	using ChannelMap = std::map<int, Channel*>;
	ChannelMap m_channels;
private:
	EventLoop* m_ownerLoop;

};

}
}
#endif

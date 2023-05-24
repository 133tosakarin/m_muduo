#ifndef DC_NET_CHANNEL_H
#define DC_NET_CHANNEL_H

#include "dc/base/noncopyable.h"
#include "dc/base/timeStamp.h"

#include <functional>
#include <memory>


namespace dc
{
namespace net
{
class EventLoop;
using std::string;

/*
 *A selectable I/O channel.

 This class doesn't own thefile descriptor.
 The file descriptor could be a socket,
 an eventfd, a timerfd, or a signalfd
 * */
class Channel : noncopyable
{
	
public:
	using ReadEventCallback = std::function<void(Timestamp)>;
	using EventCallback = std::function<void()>;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent(Timestamp receiveTime);
	void setReadCallback(ReadEventCallback cb) { m_readCallback = cb; }
	void setWriteCallback(EventCallback cb) { m_writeCallback = cb; } 
	void setErrorCallback(EventCallback cb) { m_errorCallback = cb; } 
	void setCloseCallback(EventCallback cb) { m_closeCallback = cb; }

	//Tie this channel to owner object managed by shared_ptr
	//prevent the owner object being destroyed in handleEvent
	void tie(const std::shared_ptr<void>&);

	int fd() const { return m_fd; };
	int events() const { return m_events; }
	void set_revents(int revt) { m_revents = revt; } //used by poller
	bool isNoneEvent() const { return m_events == kNoneEvent; } 

	void enableReading() { m_events |= kReadEvent; update(); }
	void disableReading() { m_events &= ~kReadEvent; update(); }

	void enableWriting() { m_events |= kWriteEvent; update(); }
	void disableWriting() { m_events &= ~kWriteEvent; update(); }

	void disableAll() { m_events = kNoneEvent; update(); }

	bool isWriting() const { return m_events & kWriteEvent; }
	bool isReading() const { return m_events & kReadEvent; }

	int index() { return m_index; }
	void set_index(int idx) { m_index =idx; }

	//for debug
	string reventsToString() const;
	string eventsToString() const;

	void doNotLogHup() { is_loghup = false; } 

	EventLoop* ownerLoop() { return m_loop; }
	void remove();


private:
	static string eventsToString(int fd, int ev);
	void update();
	void handleEventWithGuard(Timestamp receiveTime);
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	EventLoop* m_loop;
	const int m_fd;
	int m_events;
	int m_revents;
	int m_index;
	bool is_loghup;

	std::weak_ptr<void> m_tie;//to bind Tcpconn

	bool is_tied;
	bool is_eventHandling;
	bool is_addedToLoop;
	ReadEventCallback m_readCallback;
	EventCallback m_writeCallback;
	EventCallback m_closeCallback;
	EventCallback m_errorCallback;

};


}

}
#endif

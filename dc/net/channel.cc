#include "dc/net/channel.h"
#include "dc/net/eventLoop.h"
#include "dc/base/logging.h"


#include <sstream>
#include <poll.h>

namespace dc
{
namespace net
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;


Channel::Channel(EventLoop* loop, int fd)
	: m_loop(loop),
	  m_fd(fd),
	  m_events(0),
	  m_revents(0),
	  m_index(-1),
	  is_loghup(true),
	  is_tied(false),
	  is_eventHandling(false),
	  is_addedToLoop(false)
{


}

Channel::~Channel()
{
	assert(!is_eventHandling);
	assert(!is_addedToLoop);
	if(m_loop->isInLoopThread())
	{

		assert(!m_loop->hasChannel(this));
	}

}


void Channel::tie(const std::shared_ptr<void>& obj)
{
	m_tie = obj;
	is_tied = true;
}


void Channel::update()
{
	is_addedToLoop = true;
	m_loop->updateChannel(this);
}

void Channel::remove()
{
	assert(isNoneEvent());
	is_addedToLoop = false;
	m_loop->removeChannel(this);
}

void Channel::handleEvent( Timestamp receiveTime)
{
	std::shared_ptr<void> guard;
	if(is_tied)
	{
		guard = m_tie.lock();
		if(guard)
		{
			handleEventWithGuard(receiveTime);
		}
	
	}
	else
	{
		handleEventWithGuard(receiveTime);	
	}

}
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	is_eventHandling = true;
	LOG_TRACE << reventsToString();
	if( (m_revents & POLLHUP ) && !(m_revents & POLLIN ))
	{
		if(is_loghup)
		{
			LOG_WARN << "never go to here unless error happend or be closed" << "fd = " << m_fd << " Channel::handle_event() POLLHUP";	
		}
		if(m_closeCallback) m_closeCallback();
	
	}
	if( m_revents & POLLNVAL)
	{
		LOG_WARN << "fd = " << m_fd << " Channel::handle_event() POLLINVAL";	
	}
	if( m_revents & (POLLERR | POLLNVAL))
	{
		if(m_errorCallback) m_errorCallback();
	}
	if( m_events & (POLLIN | POLLPRI | POLLRDHUP))
	{
		if(m_readCallback) m_readCallback(receiveTime);
	}
	if( m_events & POLLOUT )
	{
		if( m_writeCallback ) m_writeCallback();
	}

	is_eventHandling = false;
}
string Channel::reventsToString() const
{
	return eventsToString(m_fd, m_revents);
}
string Channel::eventsToString() const
{
	return eventsToString(m_fd, m_events);
}

string Channel::eventsToString(int fd, int ev)
{
	std::ostringstream oss;
	oss << fd << ": ";
	if( ev & POLLIN )
		oss << "IN ";
	if( ev & POLLOUT )
		oss << "OUT ";
	if( ev & POLLPRI )
		oss << "PRI ";
	if( ev & POLLHUP )
		oss << "HUP ";
	if( ev & POLLRDHUP )
		oss << "RDHUP ";
	if( ev & POLLERR )
		oss << "ERR ";
	if( ev & POLLNVAL )
		oss << "NVAL ";
	return oss.str();
}




}//namespace net

}//namespace dc

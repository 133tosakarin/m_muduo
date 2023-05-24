#include "dc/net/connector.h"
#include "dc/base/logging.h"
#include "dc/net/eventLoop.h"
#include "dc/net/channel.h"
#include "dc/net/socketsOps.h"
#include "dc/net/timerId.h"
#include <errno.h>

using namespace dc;
using namespace dc::net;


const int Connector::kMaxRetryDelayMs;
const int Connector::kInitRetryDelayMs;
Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
	: m_loop( loop ),
	  m_serverAddr(serverAddr),
	  is_connect(false),
	  m_state( kDisconnected ),
	  m_retryDelayMs(kInitRetryDelayMs)
{
	LOG_DEBUG << "ctor[" << this << "]";
}
Connector::~Connector()
{
	LOG_DEBUG << "dtor[" << this << "]";
	assert(!m_channel);
}

void Connector::start()
{
	is_connect = true;
	m_loop->runInLoop(std::bind(&Connector::startInLoop, this));
}
void Connector::startInLoop()
{
	m_loop->assertInLoopThread();
	assert(m_state == kDisconnected);
	if( is_connect )
	{
		connect();
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}

void Connector::stop()
{
	is_connect = false;
	m_loop->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
	m_loop->assertInLoopThread();
	if( m_state == kConnecting)
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);
	}
}

void Connector::connect()
{
	int sockfd = sockets::createNonblockingOrDie(m_serverAddr.family());
	int ret = sockets::connect(sockfd, m_serverAddr.getSockaddr());
	int saveErrno = (ret == 0 ) ? 0 : errno;
	switch( saveErrno )
	{
		case 0:
		case EINPROGRESS:
		case EINTR:
		case EISCONN:
			connecting(sockfd);
			break;

		case EAGAIN:
		case EADDRINUSE:
		case ECONNREFUSED:
		case ENETUNREACH:
			retry(sockfd);
			break;

		case EACCES:
		case EPERM:
		case EAFNOSUPPORT:
		case EALREADY:
		case EBADF:
		case EFAULT:
		case ENOTSOCK:
			LOG_SYSERR << "connect error in COnnector::startInLoop " << saveErrno;
			sockets::close(sockfd);
			break;
		default:
			LOG_SYSERR << "unexpected error in Connector::startInLoop " << saveErrno;
			sockets::close(sockfd);
			break;
	}
}


void Connector::restart()
{
	m_loop->assertInLoopThread();
	setState(kDisconnected);
	m_retryDelayMs = kInitRetryDelayMs;
	is_connect = true;
	startInLoop();
}


void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	assert(!m_channel);
	m_channel.reset(new Channel(m_loop, sockfd));
	m_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
	m_channel->setErrorCallback(std::bind(&Connector::handleError, this));
	m_channel->enableWriting();
}

int Connector::removeAndResetChannel()
{
	m_channel->disableAll();
	m_channel->remove();
	int sockfd = m_channel->fd();
	m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockfd;
}


void Connector::resetChannel()
{
	m_channel.reset();
}

void Connector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite " << m_state;

	if( m_state == kConnecting )
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		if( err )
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = " 
					 << err << " " << strerror_tl(err);
			retry(sockfd);
		}
		else if( sockets::isSelfConnect(sockfd))
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry(sockfd);
		}
		else
		{
			setState(kConnected);
			if( is_connect )
			{
				m_newConnectionCallback(sockfd);
			}
			else
			{
				sockets::close(sockfd);
			}
		}
	}
	else
	{
		assert(m_state == kDisconnected);
	}
}


void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError state = " << m_state;
	if( m_state == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
		retry(sockfd);
	}
}


void Connector::retry(int sockfd)
{
	sockets::close(sockfd);
	setState(kDisconnected);
	if( is_connect)
	{
		LOG_INFO << "Connector::retry - Retry connecting to " << m_serverAddr.toIpPort()
				 << " in " << m_retryDelayMs << " milliseconds. ";
		m_loop->runAfter(m_retryDelayMs / 1000.0, 
						std::bind(&Connector::startInLoop, shared_from_this()));
		m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}

#include "dc/net/tcpConnection.h"
#include "dc/base/weakCallback.h"
#include "dc/net/channel.h"
#include "dc/net/eventLoop.h"
#include "dc/base/logging.h"
#include "dc/net/socket.h"
#include "dc/net/socketsOps.h"
#include "dc/net/timerId.h"
#include <errno.h>


using namespace dc;
using namespace dc::net;


void dc::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
			  << conn->peerAddress().toIpPort() << " is "
			  << (conn->connected() ? "UP" : "DOWN");
}

void dc::net::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)
{
	buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
							 const string& nameArg,
							 int sockfd,
							 const InetAddress& localAddr,
							 const InetAddress& peerAddr)
	: m_loop(loop),
	  m_name(nameArg),
	  m_state(kConnecting),
	  is_reading(false),
	  m_socket(new Socket(sockfd)),
	  m_channel(new Channel(loop, sockfd)),
	  m_localAddr(localAddr),
	  m_peerAddr(peerAddr),
	  m_highWaterMark(64*1024*1024)
{
	m_channel->setReadCallback( std::bind(&TcpConnection::handleRead, this, _1) );
	m_channel->setWriteCallback( std::bind(&TcpConnection::handleWrite, this));
	m_channel->setCloseCallback( std::bind(&TcpConnection::handleClose, this));
	m_channel->setErrorCallback( std::bind(&TcpConnection::handleError, this));
	LOG_DEBUG << "TcpConenction::ctor[" << m_name << "] at " << this << " fd = " << sockfd;
}
TcpConnection::~TcpConnection()
{
	LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
			  << " fd = " << m_channel->fd()
			  << " state = " << stateToString();
	assert(m_state == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
	return m_socket->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const 
{
	char buf[1024];
	buf[0] = '\0';
	m_socket->getTcpInfoString( buf, sizeof buf);
	return buf;
}

void TcpConnection::send(const void* data, int len)
{
	send(StringPiece(static_cast<const char* >(data), len));
}
void TcpConnection::send(const StringPiece& message)
{
	if( m_state == kConnected)
	{
		if(m_loop->isInLoopThread())
		{
			sendInLoop(message);
		}
		else
		{
			void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
			m_loop->runInLoop( std::bind(fp, this, message.as_string()));
		}
	}
}

void TcpConnection::send(Buffer* buf)
{
	if( m_state == kConnected)
	{
		if(m_loop->isInLoopThread())
		{
			sendInLoop(buf->peek(), buf->readableBytes());
			buf->retrieveAll();
		}
		else
		{
			void (TcpConnection::*fp)(const StringPiece& message)  = &TcpConnection::sendInLoop;
			m_loop->runInLoop( std::bind(fp, this, buf->retrieveAllAsString()));
		}
	}
}

  void TcpConnection::send(string&& message)
{
	if(m_state == kConnected )
	{
		if( m_loop->isInLoopThread())
		{
			sendInLoop(std::move(message));
		}
		else
		{
			void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
			m_loop->runInLoop(std::bind(fp, this, message));
		}
	}
}

void TcpConnection::sendInLoop(const StringPiece& message)
{
	sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(string&& message)
{
	sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len )
{
	m_loop->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if (m_state == kDisconnected)
	{
		LOG_WARN << "disconneted, give up writing";
		return;
	}
	//if no thing in output queue, try wriing directly
	if(!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0 )
	{
		nwrote = sockets::write(m_channel->fd(), data, len );
		if( nwrote >= 0 )
		{
			remaining  = len - nwrote;
			if( remaining == 0 && m_writeCompleteCallback)
			{
				m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
			}
		}
		else
		{
			nwrote = 0;
			if( errno != EWOULDBLOCK )
			{
				LOG_SYSERR << "TcpConnection::sendInLoop";
				if( errno == EPIPE || errno == ECONNRESET)
				{
					faultError = true;
				}
			}
		}
	}

	assert(remaining <= len );
	if( !faultError && remaining > 0 )
	{
		size_t oldlen = m_outputBuffer.readableBytes();
		if( oldlen + remaining >= m_highWaterMark 
				&& oldlen < m_highWaterMark
				&& m_highWaterMarkCallback )
		{
			m_loop->queueInLoop( std::bind(m_highWaterMarkCallback, shared_from_this(), oldlen + remaining));
		}
		m_outputBuffer.append(static_cast<const char* >(data) + nwrote, remaining);
		{
			if(!m_channel->isWriting())
			{
				m_channel->enableWriting();
			}
		}
	}
}

void TcpConnection::shutdown()
{
	if( m_state == kConnected)
	{
		setState(kDisconnecting);
		m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}


void TcpConnection::shutdownInLoop()
{
	m_loop->assertInLoopThread();
	{
		if(!m_channel->isWriting())
		{
			m_socket->shutdownWrite();
		}
	}
}

void TcpConnection::forceClose()
{
	if( m_state == kConnected || m_state == kDisconnecting)
	{
		setState(kDisconnecting);
		m_loop->queueInLoop( std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
	}
}

void TcpConnection::forceCloseWithDelay(double seconds)
{
	if( m_state == kConnected || m_state == kDisconnecting)
	{
		setState(kDisconnecting);
		m_loop->runAfter(seconds, makeWeakCallback(shared_from_this(), &TcpConnection::forceClose));
	}
}

void TcpConnection::forceCloseInLoop()
{
	m_loop->assertInLoopThread();
	if( m_state == kConnected || m_state == kDisconnecting)
	{
		handleClose();
	}
}


const char* TcpConnection::stateToString() const 
{
	switch( m_state)
	{
		case kDisconnected:
			return "kDisconncted";
		case kConnecting:
			return "kConnecting";
		case kConnected:
			return "kConnected";
		case kDisconnecting:
			return "kDisconnecting";
		default:
			return "unknown state";
	}
}

void TcpConnection::setTcpNoDelay(bool on)
{
	m_socket->setTcpNoDelay(on);
}


void TcpConnection::startRead()
{
	m_loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
	m_loop->assertInLoopThread();
	if( !is_reading || !m_channel->isReading())
	{
		m_channel->enableReading();
		is_reading = true;
	}
}

void TcpConnection::stopRead()
{
	m_loop->runInLoop( std::bind(&TcpConnection::stopReadInLoop, this));
}
void TcpConnection::stopReadInLoop()
{
	m_loop->assertInLoopThread();
	if( is_reading || m_channel->isReading())
	{
		m_channel->disableReading();
		is_reading = false;
	}
}

void TcpConnection::connectEstablished()
{
	m_loop->assertInLoopThread();
	assert(m_state == kConnecting);
	setState(kConnected);
	m_channel->tie(shared_from_this());
	m_channel->enableReading();
	m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
	m_loop->assertInLoopThread();
	if( m_state == kConnected)
	{
		setState(kDisconnected);
		m_channel->disableAll();
		m_connectionCallback(shared_from_this());
	}
	m_channel->remove();
}

void TcpConnection::handleRead( Timestamp receiveTime )
{
	m_loop->assertInLoopThread();
	int saveErrno = 0;
	ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);
	if( n > 0 )
	{
		m_messageCallback( shared_from_this(), &m_inputBuffer, receiveTime);
	}
	else if( n == 0 )
	{
		handleClose();
	}
	else
	{
		errno = saveErrno;
		LOG_SYSERR << "TcpConnection::handleRead";
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	m_loop->assertInLoopThread();
	if (m_channel->isWriting())
	{
		ssize_t n = sockets::write(m_channel->fd(),
									m_outputBuffer.peek(),
									m_outputBuffer.readableBytes());
		if( n > 0 )
		{
			m_outputBuffer.retrieve(n);
			if ( m_outputBuffer.readableBytes() == 0 )
			{
				m_channel->disableWriting();
				if( m_writeCompleteCallback )
				{
					m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
				}
				if( m_state == kDisconnecting)
				{
					shutdownInLoop();
				}
			}
		}
		else
		{
			LOG_SYSERR << "TcpConnection::handleWrite";
		}
	}
	else
	{
		LOG_TRACE << "Connection fd = " << m_channel->fd()
				  << " is down, no more writing";
	}
}

void TcpConnection::handleClose()
{
	m_loop->assertInLoopThread();
	LOG_TRACE << "fd = " << m_channel->fd() << " state = " << stateToString();
	assert(m_state == kConnected || m_state == kDisconnecting);
	setState(kDisconnected);
	m_channel->disableAll();
	TcpConnectionPtr guardThis(shared_from_this());
	m_connectionCallback(guardThis);
	m_closeCallback(guardThis);
}

void TcpConnection::handleError()
{
	int err = sockets::getSocketError( m_channel->fd() );
	LOG_ERROR << "TcpConnection::handleError [" << m_name
			 << "] - SO_ERROR = "  << err << " " << strerror_tl(err);
}






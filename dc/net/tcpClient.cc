#include "dc/net/tcpClient.h"
#include "dc/net/connector.h"

#include "dc/net/eventLoop.h"
#include "dc/net/socketsOps.h"
#include "dc/net/timerId.h"
#include "dc/base/logging.h"

#include <cstdio>

using namespace dc;
using namespace dc::net;

namespace dc
{
namespace net
{
namespace detail
{

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
	loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{

	connector->stop();
}

}// namespace detail

}//namespace net

}//namespace dc

TcpClient::TcpClient(EventLoop* loop,
					const InetAddress& serverAddr,
					const string& nameArg)
	: m_loop( loop ),
	  m_connector(new Connector(loop, serverAddr)),
	  m_name(nameArg),
	  m_connectionCallback(defaultConnectionCallback),
	  m_messageCallback(defaultMessageCallback),
	  is_retry(false),
	  is_connect(true),
	  m_nextConnId(1)
{
	m_connector->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, _1));
	LOG_INFO << "TcpClient::TcpClient[" << m_name
			 << "] - connector " << get_pointer(m_connector);
}
TcpClient::~TcpClient()
{
	LOG_INFO << "TcpClient::~TcpClient[" << m_name
			 << "] - connector " << get_pointer(m_connector);
	TcpConnectionPtr conn;
	bool unique = false;
	{
		MutexLockGuard lock(m_mutex);
		unique = m_connection.unique();
		conn = m_connection;
	}
	if( conn )
	{
		assert(m_loop == conn->getLoop());
		CloseCallback cb = std::bind(&detail::removeConnection, m_loop, _1);
		m_loop->runInLoop( std::bind(&TcpConnection::setCloseCallback, conn, cb));
		if(unique)
		{
			conn->forceClose();
		}
	}
	else
	{
		m_connector->stop();
		m_loop->runAfter(1.0, std::bind(&detail::removeConnector, m_connector));
	}
}

void TcpClient::connect()
{
	LOG_INFO << "TcpClient::connect[" << m_name << "] - connecting to "
			 << m_connector->serverAddress().toIpPort();
	is_connect = true;
	m_connector->start();
}

void TcpClient::disconnect()
{
	is_connect = false;
	{
		MutexLockGuard lock(m_mutex);
		if(m_connection)
		{
			m_connection->shutdown();
		}
	}
}
void TcpClient::stop()
{
	is_connect = false;
	m_connector->stop();
}

void TcpClient::newConnection( int sockfd )
{
	m_loop->assertInLoopThread();
	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), m_nextConnId);
	++m_nextConnId;
	string connName = m_name + buf;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));

	TcpConnectionPtr conn(new TcpConnection(m_loop,
										     connName,
											 sockfd,
											 localAddr,
											 peerAddr));
	conn->setConnectionCallback(m_connectionCallback);
	conn->setMessageCallback(m_messageCallback);
	conn->setWriteCompleteCallback(m_writeCompleteCallback);
	conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
	{
		MutexLockGuard lock(m_mutex);
		m_connection = conn;
	}
	conn->connectEstablished();
}


void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	m_loop->assertInLoopThread();
	assert(m_loop == conn->getLoop());

	{
		MutexLockGuard lock(m_mutex);
		assert(m_connection == conn);
		m_connection.reset();
	}

	m_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	if( is_retry && is_connect)
	{
		LOG_INFO << "TcpClient::connect[" << m_name << "] - reconnecting to " 
				 << m_connector->serverAddress().toIpPort();
		m_connector->restart();
	}
}

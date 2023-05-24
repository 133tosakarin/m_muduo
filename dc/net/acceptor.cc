#include "dc/net/acceptor.h"
#include "dc/base/logging.h"
#include "dc/net/eventLoop.h"
#include "dc/net/inetAddress.h"
#include "dc/net/socketsOps.h"


#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

using namespace dc;
using namespace dc::net;



Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) 
	: m_loop( loop ),
	  m_acceptSocket(sockets::createNonblockingOrDie(listenAddr.family())),
	  m_acceptChannel(loop, m_acceptSocket.fd()),
	  m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC )),
	  is_listening(false)
{
	assert(m_idleFd >= 0 );
	m_acceptSocket.setReuseAddr(true);
	m_acceptSocket.setReusePort(reuseport);
	m_acceptSocket.bindAddress(listenAddr);
	m_acceptChannel.setReadCallback( std::bind(&Acceptor::handleRead, this));

}
Acceptor::~Acceptor()
{
	m_acceptChannel.disableAll();
	m_acceptChannel.remove();
	::close(m_idleFd);
}

void Acceptor::listen()
{
	m_loop->assertInLoopThread();
	is_listening = true;
	m_acceptSocket.listen();
	m_acceptChannel.enableReading();

}

void Acceptor::handleRead()
{
	m_loop->assertInLoopThread();
	InetAddress peerAddr;
	int connfd = m_acceptSocket.accept(&peerAddr);
	if( connfd >= 0 )
	{
		if( m_newConnectionCallback )
		{
			m_newConnectionCallback(connfd, peerAddr);
		}
		else
		{
			sockets::close(connfd);
		}
	}
	else
	{
		LOG_SYSERR << "in Acceptor::handleRead";
		if( errno == EMFILE )
		{
			::close(m_idleFd);
			m_idleFd = ::accept(m_acceptSocket.fd(), nullptr, nullptr);
			::close(m_idleFd);
			m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC );
		}
	}

}


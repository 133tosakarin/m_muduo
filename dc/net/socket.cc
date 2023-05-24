#include "dc/base/logging.h"
#include "dc/net/inetAddress.h"
#include "dc/net/socketsOps.h"
#include "dc/net/socket.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> //snprintf


using namespace dc;
using namespace dc::net;
Socket::~Socket()
{
	sockets::close(m_sockfd);	

}

//return true if success
bool Socket::getTcpInfo(struct tcp_info* tcpi) const
{
	socklen_t len = sizeof(*tcpi);
	memZero(tcpi, len);
	return ::getsockopt(m_sockfd, SOL_TCP, TCP_INFO, tcpi, &len ) == 0;

}
bool Socket::getTcpInfoString(char* buf, int len) const
{
	struct tcp_info tcpi;
	bool ok = getTcpInfo(&tcpi);
	if( ok )
	{
		snprintf(buf, len, "unrecovered=%u"
				 "rot=%u ato=%u snd_mss=%u rcv_mss=%u "
				 "lost=%u retrans=%u rtt=%u rttvar=%u "
				 "sshthresh=%u cwnd=%u total_retrans=%u",
				 tcpi.tcpi_retransmits, //number of unrecovered timeouts
				 tcpi.tcpi_rto, 		//retransmit timeou in usec
				 tcpi.tcpi_ato,			//predicted tick of soft clock in usec
				 tcpi.tcpi_snd_mss,			
				 tcpi.tcpi_rcv_mss,
				 tcpi.tcpi_lost,		//lost packets
				 tcpi.tcpi_retrans,		//Retransimitted packets out
				 tcpi.tcpi_rtt,			//Smoothed round trip time in usec
				 tcpi.tcpi_rttvar,		//Medium deviation
				 tcpi.tcpi_snd_ssthresh,
				 tcpi.tcpi_snd_cwnd,
				 tcpi.tcpi_total_retrans);//Total retransimits for entire connection

	
	}
	return ok;

}

///abort if address in use
void Socket::bindAddress(const InetAddress& localaddr)
{
	sockets::bindOrDie(m_sockfd, localaddr.getSockaddr());

}
// abort if address in use
void Socket::listen()
{
	sockets::listenOrDie(m_sockfd);

}

/*
 *on success, return a non-negative integer that is a
 descriptor for the accepted socket, which has been set to non-blocking ans close-on-exec.
 *peeraddr is assigned. On error , -1 is return ed, ans *peeraddr is untouched.
 * */
int Socket::accept(InetAddress * peeraddr)
{
	struct sockaddr_in6 addr;
	memZero(&addr, sizeof addr);
	int connfd = sockets::accept(m_sockfd, &addr);
	if( connfd >= 0 )
	{
	
		peeraddr->setSockAddrInet6(addr);	
	}
	return connfd;

}

void Socket::shutdownWrite()
{
	sockets::shutdownWrite(m_sockfd);
}

//Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

// Enable/disable SO_REUSEADDR
void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
							&optval, static_cast<socklen_t>(sizeof optval));
	(void)ret;
}
//Enable/disable SO_REUSEPORT
void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int optval  = on ? 1 : 0;
	int ret = ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT,
							&optval, static_cast<socklen_t>(sizeof optval));
	if( ret < 0 && on )
	{
		LOG_SYSERR << "SO_REUSEPORT failed.";	
	
	}
#else
	if(on)
	{
		LOG_ERROR << "SO_REUSEPORT is no t supported.";	
	}
#endif
}

//enable/disable SO_KEEPALIVE
void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, 
				&optval, static_cast<socklen_t>(sizeof optval));

}


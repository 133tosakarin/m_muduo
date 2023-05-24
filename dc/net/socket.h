#ifndef DC_NET_SOCKET_H
#define DC_NET_SOCKET_H

#include "dc/base/noncopyable.h"




// in <netinet/tcp.h>
struct tcp_info;


namespace dc
{
namespace net
{

class InetAddress;

class Socket : noncopyable
{
public:
	explicit Socket(int sockfd ) : m_sockfd( sockfd )
	{
	
	}

	~Socket();

	int fd() const { return m_sockfd; }
	//return true if success	
	bool getTcpInfo(struct tcp_info* ) const;
	bool getTcpInfoString(char* buf, int len) const;

	///abort if address in use
	void bindAddress(const InetAddress& localaddr);
	// abort if address in use
	void listen();

	/*
	 *on success, return a non-negative integer that is a
	 descriptor for the accepted socket, which has been set to non-blocking ans close-on-exec.
	 *peeraddr is assigned. On error , -1 is return ed, ans *peeraddr is untouched.
	 * */
	int accept(InetAddress * peeraddr);

	void shutdownWrite();

	//Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
	void setTcpNoDelay(bool on);

	// Enable/disable SO_REUSEADDR
	void setReuseAddr(bool on);
	//Enable/disable SO_REUSEPORT
	void setReusePort(bool on);

	//enable/disable SO_KEEPALIVE
	void setKeepAlive(bool on);

private:
	int m_sockfd;

};

}

}
#endif

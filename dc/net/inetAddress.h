#ifndef DC_NET_INETADDRESS_H
#define DC_NET_INETADDRESS_H

#include "dc/base/stringPieces.h"

#include <netinet/in.h>


namespace dc
{
namespace net
{
namespace sockets
{
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

}


//This is an POD interface class.
class InetAddress
{
public:
	//Constructs an endpoint with given port number
	//Mostly used in tcpServer listening.
	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

	// Constructs an endpoint with given ip and port.
	// ip shoud be "1.2.3.4"
	InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

	// Constructs an endpoint with given struct sockaddr_in
	// mostly used when accepting new conections
	explicit InetAddress(const struct sockaddr_in& addr) : m_addr(addr)
	{
	
	
	}

	explicit InetAddress(const struct sockaddr_in6& addr6) : m_addr6(addr6)
	{
	}


	sa_family_t family() const { return m_addr.sin_family; } 
	string toIp() const;
	string toIpPort() const;
	uint16_t port() const;

	const struct sockaddr* getSockaddr()  const { return sockets::sockaddr_cast(&m_addr6); }
	void setSockAddrInet6(const struct sockaddr_in6& addr6) { m_addr6 = addr6; }
	uint32_t ipv4NetEndian() const;
	uint16_t portNetEndian() const { return m_addr.sin_port; }

	//resolvei hostname to Ip address, not changing port or sin_family
	//return true on success
	//thread safe
	
	static bool resolve(StringArg hostname, InetAddress* result);

	void setScopeId(uint32_t socpe_id);
private:
	union
	{
		struct sockaddr_in m_addr;
		struct sockaddr_in6 m_addr6;
	
	};
};


}//namespace net

}//namespace dc
#endif //DC_NET_INETADDRESS_H

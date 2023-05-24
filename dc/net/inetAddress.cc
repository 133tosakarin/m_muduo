#include "dc/net/inetAddress.h"
#include "dc/base/logging.h"
#include "dc/net/endian.h"
#include "dc/net/socketsOps.h"

#include <netdb.h>
#include <netinet/in.h>


#pragma GCC diagnostic ignored "-Wold-style-cast"

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

#pragma GCC diagnostic error "-Wold-style-cast"


using namespace dc;
using namespace dc::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6) );
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6)
{
	static_assert(offsetof(InetAddress, m_addr6) == 0, "m_addr6_offset 0");
	static_assert(offsetof(InetAddress, m_addr) == 0, "m_addr_offset 0");
	if(ipv6)
	{
		memZero(&m_addr6, sizeof m_addr6);
		m_addr6.sin6_family = AF_INET6;
		m_addr6.sin6_port = sockets::hostToNetwork16(portArg);
		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		m_addr6.sin6_addr = ip;
	
	}
	else
	{
		memZero(&m_addr, sizeof m_addr);
		m_addr.sin_family = AF_INET;
		m_addr.sin_port =sockets::hostToNetwork16(portArg);
		in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
		m_addr.sin_addr.s_addr = sockets::hostToNetwork32(ip);
	
	}

}

InetAddress::InetAddress(StringArg ip, uint16_t portArg, bool ipv6)
{
	if(ipv6 || strchr(ip.c_str(), ':'))
	{
		memZero(&m_addr6, sizeof m_addr6);
		sockets::fromIpPort(ip.c_str(), portArg, &m_addr6);
	}
	else
	{
		memZero(&m_addr, sizeof m_addr);
		sockets::fromIpPort(ip.c_str(), portArg, &m_addr);
	}

}
string InetAddress::toIpPort() const
{
	char buf[64] = "";
	sockets::toIpPort(buf, sizeof buf, getSockaddr());
	return buf;
}

string InetAddress::toIp() const
{
	char buf[64] = "";
	sockets::toIp(buf, sizeof buf, getSockaddr());
	return buf;
}

uint16_t InetAddress::port() const
{
	return sockets::networkToHost16(portNetEndian());

}

uint32_t InetAddress::ipv4NetEndian() const
{
	assert(family() == AF_INET );
	return m_addr.sin_addr.s_addr;

}

//resolvei hostname to Ip address, not changing port or sin_family
//return true on success
//thread safe

static thread_local char t_resolveBuffer[64 * 1024];
 bool InetAddress::resolve(StringArg hostname, InetAddress* result)
{
	assert( result != nullptr );
	struct hostent hent;
	struct hostent* he = nullptr;
	int herrno = 0;
	memZero(&hent, sizeof hent);
	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
	if( ret == 0 && he != nullptr )
	{
		assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
		result->m_addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
	}
	else
	{
		if( ret )
		{
			LOG_SYSERR << "InetAddress::resolve";	
		}
		return false;
	}


}

void InetAddress::setScopeId(uint32_t socpe_id)
{
	if( family() == AF_INET6 )
	{
		m_addr6.sin6_scope_id = socpe_id;	
	}
}







/*
 * =====================================================================================
 *
 *       Filename:  acceptor.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2023 11:04:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef DC_NET_ACCEPTOR_H
#define DC_NET_ACCEPTOR_H
#include "dc/net/channel.h"
#include "dc/net/socket.h"
class EventLoop;
class InetAddress;
namespace dc
{
namespace net
{
class Acceptor : noncopyable
{
public:
 using NewConnectionCallback = std::function<void (int sockfd,  const InetAddress&)>;
	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb ) { m_newConnectionCallback = cb; }
	void listen();
	bool listening() const { return is_listening; } 
private:

 	void handleRead();
	EventLoop* m_loop;
	Socket m_acceptSocket;
	Channel m_acceptChannel;
	NewConnectionCallback m_newConnectionCallback;
	int m_idleFd;
	bool is_listening;


};

}


}
#endif

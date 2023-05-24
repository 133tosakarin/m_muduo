/*
 * =====================================================================================
 *
 *       Filename:  connector.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/14/2023 12:12:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef DC_NET_CONNECTOR_H
#define DC_NET_CONNECTOR_H
#include "dc/base/noncopyable.h"
#include "dc/net/inetAddress.h"

#include <functional>
#include <memory>
namespace dc
{
namespace net
{
class Channel;
class EventLoop;
class Connector : noncopyable, public  std::enable_shared_from_this<Connector>
{
public:
	using NewConnectionCallback = std::function<void (int sockfd)>;
	Connector(EventLoop* loop, const InetAddress& serverAddr);
	~Connector();

	void setNewConnectionCallback(const NewConnectionCallback& cb) { m_newConnectionCallback = cb; } 

	void start(); // can be called in any thread
	void restart(); // must be called in loop thread
	void stop();

	const InetAddress& serverAddress() const { return  m_serverAddr; }
private:
	enum State { kDisconnected, kConnecting, kConnected };
	static const int kMaxRetryDelayMs = 30 * 1000;
	static const int kInitRetryDelayMs = 500;

	void setState(State s) { m_state = s; }
	void startInLoop();
	void stopInLoop();
	void connect();
	void connecting(int sockfd);
	void handleWrite();
	void handleError();
	void retry(int sockfd);
	int removeAndResetChannel();
	void resetChannel();
	EventLoop* m_loop;
	InetAddress m_serverAddr;
	bool is_connect;
	State m_state;
	std::unique_ptr<Channel> m_channel;
	NewConnectionCallback m_newConnectionCallback;
	int m_retryDelayMs;



};



}






}
#endif

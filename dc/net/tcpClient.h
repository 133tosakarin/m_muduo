#ifndef DC_NET_TCPCLIENT_H
#define DC_NET_TCPCLIENT_H


#include "dc/base/mutex.h"
#include "dc/net/tcpConnection.h"

namespace dc
{
namespace net
{
class Connector;

using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : noncopyable
{
public:
	TcpClient(EventLoop* loop, 
				const InetAddress& m_serverAddr,
				const string& nameArg);
	~TcpClient();

	void connect();
	void disconnect();
	void stop();

	TcpConnectionPtr connection() const
	{
		MutexLockGuard lock(m_mutex);
		return m_connection;
	}

	EventLoop* getLoop() const { return m_loop; }

	bool retry() const { return is_retry; }
	void enableRetry() { is_retry = true; }

	const string& name() const { return m_name; }
	void setConnectionCallback( ConnectionCallback cb) { m_connectionCallback = std::move(cb); }
	void setMessageCallback( MessageCallback cb ) { m_messageCallback = std::move(cb); }
	void setWriteCompleteCallback( WriteCompleteCallback cb ) { m_writeCompleteCallback = std::move(cb); }
private:
	void newConnection( int sockfd);
	void removeConnection(const TcpConnectionPtr& conn);

	EventLoop* m_loop;
	ConnectorPtr m_connector;
	const string m_name;
	ConnectionCallback m_connectionCallback;
	MessageCallback m_messageCallback;
	WriteCompleteCallback m_writeCompleteCallback;
	bool is_retry;
	bool is_connect;

	int m_nextConnId;
	mutable MutexLock m_mutex;
	TcpConnectionPtr m_connection GUARDED_BY(m_mutex);
};



}


}
#endif

/*
 * =====================================================================================
 *
 *       Filename:  tcpServer.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2023 12:52:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef DC_NET_TCPSERVER_H
#define DC_NET_TCPSERVER_H
#include "dc/base/atomic.h"
#include "dc/base/types.h"
#include "dc/net/tcpConnection.h"

#include <map>
namespace dc
{
namespace net
{
class EventLoop;
class Acceptor;
class EventLoopThreadPool;
class TcpServer : noncopyable
{
public:
	using ThreadInitCallback = std::function<void (EventLoop*)>;
	enum Option
	{
		kNoReusePort,
		kReusePort,
	};

	TcpServer(EventLoop* loop,
			  const InetAddress& listenAddr,
			  const string& nameArg,
			  Option option = kNoReusePort);
	~TcpServer();

	const string& ipPort() const { return m_ipPort; }
	const string& name() const { return m_name; } 
	EventLoop* getLoop() const { return m_loop; }


	// set the num of threads for handling input
	// always accepts new connection in loop's thread.
	// Must be called before start
	// 0 means all I/O in loop's thread, no thread will created, this is default value
	// 1 means all I/O in another thread
	// N means a thread pool with N threads, new connctions are assigned on a round-robin basis
	
	void setThreadNum(int numThreads);
	void setThreadInitCallback(const ThreadInitCallback& cb) { m_threadInitCallback = cb; }

	std::shared_ptr<EventLoopThreadPool> threadPool() { return m_threadPool; }

	//start the server if it's not listening
	//it's harmless to call it multiple times.
	//thread safe
	void start();

	void setConnectionCallback(const ConnectionCallback& cb){ m_connectionCallback = cb; }
	void setMessageCallback(const MessageCallback& cb){ m_messageCallback = cb; }
	void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }

private:
	void newConnection(int sockfd,  const InetAddress& peerAddr);
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);
	using ConnectionMap = std::map<string, TcpConnectionPtr>;

	EventLoop* m_loop; // the acceptor loop
	const string m_ipPort;
	const string m_name;
	std::unique_ptr<Acceptor> m_acceptor;
	std::shared_ptr<EventLoopThreadPool> m_threadPool;
	ConnectionCallback m_connectionCallback;
	MessageCallback m_messageCallback;
	WriteCompleteCallback m_writeCompleteCallback;
	ThreadInitCallback m_threadInitCallback;
	AtomicInt32 is_started;

	int m_nextConnId;
	ConnectionMap m_connections;
};
}//namespace net
}//namespace dc
#endif

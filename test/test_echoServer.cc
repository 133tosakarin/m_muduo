
#include	<stdlib.h>
#include "dc/net/tcpServer.h"
#include "dc/base/logging.h"
#include "dc/base/thread.h"
#include "dc/net/eventLoop.h"
#include "dc/net/inetAddress.h"

#include <utility>
#include <cstdio>
#include <unistd.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
using namespace dc;
using namespace dc::net;
int numThreads;
class EchoServer
{
public:
	EchoServer(EventLoop* loop,  const InetAddress& listenAddr)
		: loop_(loop),
		  server_(loop, listenAddr, "EchoServer")
	{
		server_.setConnectionCallback( std::bind(&EchoServer::onConnection, this, _1));
		server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
		server_.setThreadNum(numThreads);
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_TRACE << conn->peerAddress().toIpPort() << " - > "
				  << conn->localAddress().toIpPort() << " is "
				  << (conn->connected() ? "UP" : "DOWN");
		LOG_INFO << conn->getTcpInfoString();
		conn->send(string("hello\n"));
	}

	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
	{
		string msg(buf->retrieveAllAsString());
		LOG_TRACE << conn->name() << " recb " << msg.size() << " bytes at " << time.toString();
		if( msg == "exit\n")
		{
			conn->send(StringPiece("bye\n"));
			conn->shutdown();
		}
		if(msg == "quit\n")
		{
			loop_->quit();

		}
		conn->send(msg);
	}

	EventLoop* loop_;
	TcpServer server_;




};

	int
main ( int argc, char *argv[] )
{
	g_logLevel = Logger::TRACE;
	numThreads = atoi(argv[1]);
	EventLoop loop;
	InetAddress listenAddr(12345);
	EchoServer echoServer(&loop, listenAddr);
	echoServer.start();
	loop.loop();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

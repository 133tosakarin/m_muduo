#ifndef DC_NET_HTTP_HTTPSERVER_H
#define DC_NET_HTTP_HTTPSERVER_H

#include "dc/net/tcpServer.h"

namespace dc
{
namespace net
{
class HttpRequest;
class HttpResponse;
class HttpServer : noncopyable
{
public:
	using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;
	HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, TcpServer::Option option = TcpServer::kNoReusePort);
	~HttpServer() = default;

	EventLoop* getLoop() const { return server_.getLoop(); }

	void start();

	void setThreadNum(int numThreads)
	{
		server_.setThreadNum(numThreads);
	}

	void setHttpCallback(const HttpCallback& cb)
	{
		httpCallback_ = cb;
	}
private:

	void onRequest(const TcpConnectionPtr&, const HttpRequest&);
	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
	TcpServer server_;
	HttpCallback httpCallback_;
};


}
}


#endif









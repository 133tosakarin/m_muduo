#ifndef DC_NET_TCPCONNECTION_H
#define DC_NET_TCPCONNECTION_H

#include "dc/base/noncopyable.h"
#include "dc/base/stringPieces.h"
#include "dc/base/types.h"
#include "dc/net/callbacks.h"
#include "dc/net/buffer.h"
#include "dc/net/inetAddress.h"


#include <memory>
#include <any>

struct tcp_info;

namespace dc
{
namespace net
{
class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop,
				  const string& name,
				  int sockfd,
				  const InetAddress& localAddr,
				  const InetAddress& peerAddr);
	~TcpConnection();
	EventLoop* getLoop() const { return m_loop; }
	const string& name() const { return m_name; }
	const InetAddress& localAddress() const { return m_localAddr; }
	const InetAddress& peerAddress() const { return m_peerAddr; }
	bool connected() const { return m_state == kConnected; }
	bool disconnected() const { return m_state == kDisconnected; }

	//return true if success;
	bool getTcpInfo(struct tcp_info* ) const;
	string getTcpInfoString() const;

	void send(const void* mesage, int len);
	void send(string&& mesage);
	void send(const StringPiece& mesage);
	void send(Buffer* message);
	void send(Buffer&& message);
	void shutdown(); //not thread safe, no simultaneous calling
	void forceClose();
	void forceCloseWithDelay(double seconds);
	void setTcpNoDelay(bool on);
	void startRead();
	void stopRead();
	bool isReading() const { return is_reading; }

	void setContext(const std::any& context)
	{
		m_context = context;
	}
	const std::any& getContext() const
	{
		return m_context;
	}
	std::any* getMutableContext()
	{
		return &m_context;
	}
	void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = cb; }
	void setMessageCallback(const MessageCallback& cb){ m_messageCallback = cb;}
	void setWriteCompleteCallback( const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark){ m_highWaterMarkCallback = cb, m_highWaterMark = highWaterMark; }
	void setCloseCallback(const CloseCallback& cb){ m_closeCallback = cb; }
	Buffer* inputBuffer()
	{
		return &m_inputBuffer;
	}
	Buffer* outputBuffer()
	{
		return &m_outputBuffer;
	}
	void connectEstablished();
	void connectDestroyed();
private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleError();
	void handleClose();
	void sendInLoop( const StringPiece& message );
	void sendInLoop(string&& message);
	void sendInLoop(const void* message, size_t len);
	void shutdownInLoop();
	void forceCloseInLoop();
	void setState(StateE s) { m_state = s; }
	const char* stateToString() const;
	void startReadInLoop();
	void stopReadInLoop();
	
	EventLoop* m_loop;
	const string m_name;
	StateE m_state;
	bool is_reading;

	std::unique_ptr<Socket> m_socket;
	std::unique_ptr<Channel> m_channel;
	const InetAddress m_localAddr;
	const InetAddress m_peerAddr;
	ConnectionCallback m_connectionCallback;
	MessageCallback m_messageCallback;
	WriteCompleteCallback m_writeCompleteCallback;
	HighWaterMarkCallback m_highWaterMarkCallback;
	CloseCallback m_closeCallback;
	size_t m_highWaterMark;
	Buffer m_inputBuffer;
	Buffer m_outputBuffer;
	std::any m_context;

};
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}// namespace net

}//namespace dc
#endif

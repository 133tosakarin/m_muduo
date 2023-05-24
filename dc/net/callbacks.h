#ifndef DC_NET_CALLBACKS_H
#define DC_NET_CALLBACKS_H

#include "dc/base/timeStamp.h"
#include "dc/base/types.h"
#include <cassert>
#include <functional>
#include <memory>


namespace dc
{
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// should really belong to base/types.h, but <memory> is no t included there.

template<typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr)
{
	return ptr.get();
}

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
	return ptr.get();
}

//Adapted from google-protobuf stubs/common.h

template<typename To, typename From>
inline std::shared_ptr<To> down_pointer_cast(const std::shared_ptr<From>& f)
{
	if(false)
	{
		implicit_cast<From*, To*>(0);
	}

#ifndef NDEBUG
	assert(f == nullptr || dynamic_cast<To*>(get_pointer(f) != nullptr));
#endif
	return std::static_pointer_cast<To>(f);
}


namespace net
{
// all client visible callbacks go here.
class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using CloseCallback = std::function<void (const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void (const TcpConnectionPtr&, size_t )>;

// the data has been read to (buf, len)
using MessageCallback = std::function<void(const TcpConnectionPtr&,
											Buffer*,
											Timestamp receiveTime)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
								Buffer* buffer, Timestamp receiveTime);

}	//namespace net
}	//namespace dc
#endif //DC_NET_CALLBACKS_H

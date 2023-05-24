#ifndef DC_NET_HTTP_HTTPCONTEXT_H
#define DC_NET_HTTP_HTTPCONTEXT_H

#include "dc/net/http/httpRequest.h"

namespace dc
{
namespace net
{
class Buffer;

class HttpContext
{
public:
	enum HttpRequestPaserState
	{
		kExpectRequestLine,
		kExpectHeaders,
		kExpectBody,
		kGotAll,
	};
	HttpContext() : state_(kExpectRequestLine)
	{

	}

	bool parseRequest(Buffer* buf, Timestamp receiveTime);

	bool gotAll() const 
	{
		return state_ == kGotAll;
	}

	void reset()
	{
		state_ = kExpectRequestLine;
		HttpRequest dummy;
		request_.swap(dummy);
	}
	const HttpRequest& request()const  { return request_; }
	HttpRequest& request() { return request_; }
private:

	bool processRequestLine(const char* begin, const char* end);
	HttpRequestPaserState state_;
	HttpRequest request_;

};
}//naemspace net 
}// naemspace dc
#endif

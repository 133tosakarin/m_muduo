#ifndef DC_NET_HTTP_REQUEST_H
#define DC_NET_HTTP_REQUEST_H

#include "dc/base/timeStamp.h"
#include "dc/base/types.h"

#include <map>
#include <assert.h>
#include <cstdio>
namespace dc
{
namespace net
{
class HttpRequest
{

public:
	enum Method
	{
		kInvalid, kGet, kPost, kDelete, kPut, kHead
	};

	enum Version
	{
		kUnknown, kHttp10, kHttp11
	};

	HttpRequest() :  method_(kInvalid), version_(kUnknown)
	{

	}
	
	void setVersion(Version v)
	{
		version_ = v;
	}
	Version getVersion() const { return version_; }
	bool setMethod(const char* start, const char* end)
	{
		assert(method_ == kInvalid);
		string m(start, end);
		if( m == "GET" )
		{
			method_ = kGet;
		}
		else if( m == "POST" )
		{
			method_ = kPost;
		}
		else if( m == "PUT" )
		{
			method_ = kPut;
		}
		else if( m == "DELETE" )
		{
			method_ = kDelete;
		}
		else
		{
			method_ = kInvalid;
		}
		return method_ != kInvalid;
	}
	Method method() const
	{
		return method_;
	}

	const char* methodString() const
	{
		const char* result = "UNKNOWN";
		switch (method_)
		{
			case kGet:
				result = "GET";
				break;
			case kPost:
				result =  "POST";
				break;
			case kPut:
				result = "PUT";
				break;
			case kDelete:
				result = "DELETE";
				break;
			case kHead:
				result = "HEAD";
				break;
			default:
				break;
		}
		return result;
	}
	void setPath(const char* start, const char* end)
	{
		path_.assign(start, end);
	}

	const string& path() const { return path_; }
	void setQuery( const char* start, const char* end)
	{
		query_.assign(start, end);
	}

	const string& query() const { return query_; }

	void setReceiveTime( Timestamp t )
	{
		receiveTime_ = t;
	}

	Timestamp receiveTime() const
	{
		return receiveTime_;
	}

	void addHeader(const char* start, const char* colon, const char* end)
	{
		string field(start, colon);
		++colon;
		while(colon < end && isspace(*colon))
		{
			++colon;
		}
		string value(colon, end);
		while(!value.empty() && isspace(value[value.size() - 1]))
		{
			value.resize(value.size() -  1);
		}

		headers_[field]  = value;
				
	}

	string getHeader(const string& field) const
	{
		string result;
		auto it = headers_.find(field);
		if( it != headers_.end() )
		{
			result = it->second;
		}
		return result;
	}
	const std::map<string,string>& headers() const
	{
		return headers_;
	}

	void swap(HttpRequest& that)
	{
		std::swap(method_, that.method_);
		std::swap(version_, that.version_);
		path_.swap(that.path_);
		query_.swap(that.query_);
		receiveTime_.swap(that.receiveTime_);
		headers_.swap(that.headers_);
	}

private:
	string path_;
	string query_;
	Timestamp receiveTime_;
	std::map<string, string> headers_;
	Method method_;
	Version version_;


};
}//namespace net
}//namespace dc
#endif

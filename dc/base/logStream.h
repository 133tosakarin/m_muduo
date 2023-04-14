#ifndef DC_BASE_LOGSTREAM_H
#define DC_BASE_LOGSTREAM_H

#include "noncopyable.h"
#include "stringPieces.h"
#include "types.h"
#include <assert.h>
#include <cstring>

namespace dc
{

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : noncopyable
{

public:
	FixedBuffer() : m_cur( m_data )
	{
		setCookie(cookieStart);
	}
	~FixedBuffer()
	{
		setCookie(cookieEnd);
	}

	void append( const char* buf, size_t len )
	{
		if(implicit_cast<size_t>(avail()) > len)
		{
			memcpy( m_cur, buf, len );
			m_cur +=len;
		}
	}

	const char* data() const { return m_data; }

	int length() const { return static_cast<int>(m_cur - m_data); }
	
	//write to m_data directly
	char* current() { return m_cur; }
	int avail() const { return static_cast<int> ( end() - m_cur ); }

	void add(size_t len) { m_cur +=len; }

	void reset() { m_cur = m_data; }

	void bzero() { memZero(m_data, sizeof m_data ); }
	
	// for used by GDB
	const char* debugString();

	void setCookie( void (*cookie)() ) { m_cookie = cookie; }

	std::string toString() const { return std::string(m_data, length()); }

	StringPiece toStringPiece() const { return StringPiece(m_data, length()); }


private:
	
	const char* end() const { return m_data + sizeof m_data; }
	static void cookieStart();
	static void cookieEnd();
	void (*m_cookie)(); 
	char m_data[SIZE];
	char* m_cur;
};

class LogStream : noncopyable
{
	using self = LogStream;
public:
	using Buffer = dc::FixedBuffer<dc::kSmallBuffer>;

	self& operator<<(bool v)
	{
		m_buffer.append(v ? "1" : "0", 1);
		return *this;
	}
	self& operator<<(short);
	self& operator<<(unsigned short);
	self& operator<<(int);
	self& operator<<(unsigned int);
	self& operator<<(long );
	self& operator<<(unsigned long );
	self& operator<<(long  long );
	self& operator<<(unsigned long  long );
	self& operator<<(const void * );
	
	self& operator<<(float v)
	{
		*this<< static_cast<double>(v);
		return *this;
	}
	self& operator<<(double);

	self& operator<<(char v)
	{
		m_buffer.append(&v, 1 );
		return *this;
	}

	self& operator<<(const char* str)
	{
		if(str)
		{
			m_buffer.append(str, strlen(str));
		}
		else
		{
			m_buffer.append("(null)", 6);
		}
		return *this;
	}

	self& operator<<(const unsigned char* str)
	{
		return operator<<(reinterpret_cast<const char*>(str)); 	
	}

	self& operator<<(const string& v)
	{
		m_buffer.append(v.c_str(), v.size());
		return *this;
	}

	self& operator<<(const StringPiece& v)
	{
		m_buffer.append(v.data(), v.size());
		return *this;
	}
	
	self& operator<<(const Buffer& v)
	{
		*this << v.toStringPiece();
		return *this;
	}

	void append(const char* data, int len) { m_buffer.append(data, len); }

	const Buffer& buffer() const { return m_buffer; }

	void resetBuffer() { m_buffer.reset(); }

private:
	void staticCheck();

	template<typename T>
	void formatInteger(T);

	Buffer m_buffer;

	static const int kMaxNumbericSize = 48;

};

class Fmt
{
public:
	template<typename T>
	Fmt(const char* fmt, T val);

	const char* data() const { return m_buf; }
	int length() const { return m_length; }
private:

	char m_buf[32];
	int m_length;
};

inline LogStream& operator<<(LogStream&s, const Fmt& fmt)
{
	s.append(fmt.data(), fmt.length());
	return s;
}

// Format quantity n in SI units (K, M, G, T, P, E).
// The returned string is atmost 5 characters long.
// Requiers n >= 0
std::string formatSI(int64_t n);

//Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
//The returned string si atmost 6 characters long.
//Requires n >= 0
std::string formatIEC(int64_t n);
}
#endif

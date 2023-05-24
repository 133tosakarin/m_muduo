#ifndef DC_NET_BUFFER_H
#define DC_NET_BUFFER_H

#include "dc/base/stringPieces.h"
#include "dc/base/types.h"
#include "dc/net/endian.h"
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

namespace dc
{
namespace net
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
/////
///// @code
///// +-------------------+------------------+------------------+
///// | prependable bytes |  readable bytes  |  writable bytes  |
///// |                   |     (CONTENT)    |                  |
///// +-------------------+------------------+------------------+
///// |                   |                  |                  |
///// 0      <=      readerIndex   <=   writerIndex    <=     size
///// @endcode
class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize)
		: m_buffers(kCheapPrepend + initialSize),
		  m_readerIndex(kCheapPrepend),
		  m_writerIndex(kCheapPrepend)
	{
		assert(readableBytes() == 0 );
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}

	void swap(Buffer& rhs)
	{
		m_buffers.swap(rhs.m_buffers);
		std::swap(m_readerIndex, rhs.m_readerIndex);
		std::swap(m_writerIndex, rhs.m_writerIndex);
	}

	size_t readableBytes() const
	{
		return m_writerIndex - m_readerIndex;
	}
	
	size_t writableBytes() const 
	{
		return m_buffers.size() - m_writerIndex;
	}

	const char* peek() const
	{
		return begin() + m_readerIndex;
	}
	
	size_t prependableBytes() const
	{
		return m_readerIndex;
	}

	const char* findCRLF() const
	{
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);// find "/r/n"
		return crlf == beginWrite() ? nullptr : crlf;
	}

	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	
	const char* findEOL() const
	{
		const void * eol = memchr(peek(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	
	}

	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}
	//retrieve returns void, to prevent
	//string str(retrieve(readableBytes()), readableBytes());
	//the evaluation of two functions are unspecified
	
	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if( len < readableBytes())
		{
			m_readerIndex += len;
		}
		else
		{
			retrieveAll();
		}
	}

	void retrieveUntil( const char* end )
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveInt64()
	{
		retrieve(sizeof(int64_t));
	}

	void retrieveInt32()
	{
		retrieve(sizeof(int32_t));
	}

	void retrieveInt16()
	{
		retrieve(sizeof(int16_t));
	}
	
	void retrieveInt8()
	{
		retrieve(sizeof(int8_t));
	}
	void retrieveAll()
	{
		m_readerIndex = kCheapPrepend;
		m_writerIndex = kCheapPrepend;
	}

	string retrieveAllAsString()
	{
		return retrieveAsString(readableBytes());
	}

	string retrieveAsString(size_t len)
	{
		assert(len <= readableBytes());
		string result(peek(), len);
		retrieve(len);
		return result;
	}

	StringPiece toStringPiece() const
	{
		return StringPiece(peek(), static_cast<int>(readableBytes()));
	}

	void append(const StringPiece& str)
	{
		append(str.data(), str.size());
	}

	void append(const char* data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWritten(len);// when writing something will call this func
	}

	void append(const void* data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	void ensureWritableBytes(size_t len)
	{
		if( writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char * beginWrite()
	{
		return begin() + m_writerIndex;
	}

	const char* beginWrite() const
	{
		return begin() + m_writerIndex;
	}

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		m_writerIndex +=len;
	}

	void unwrite(size_t len)
	{
		assert(len <= readableBytes());
		m_writerIndex -= len;
	}

	void appendInt64(int64_t x)
	{
		int64_t be64 = sockets::hostToNetwork64(x);
		append(&be64, sizeof be64);
	}

	void appendInt32(int32_t x)
	{
		int32_t be32 = sockets::hostToNetwork32(x);
		append(&be32, sizeof be32);
	}

	void appendInt16(int16_t x)
	{
		int16_t be16 = sockets::hostToNetwork16(x);
		append(&be16, sizeof be16);
	}
	
	void appendInt8(int8_t x)
	{
		append(&x, sizeof x);
	}

	int64_t readInt64()
	{
		int64_t result = peekInt64();
		retrieveInt64();
		return result;
	}

	int32_t readInt32()
	{
		int32_t result = peekInt32();
		retrieveInt32();
		return result;
	}
	int16_t readInt16()
	{
		int16_t result = peekInt16();
		retrieveInt16();
		return result;
	}
	int32_t readInt8()
	{
		int8_t result = peekInt8();
		retrieveInt8();
		return result;
	}

	//Peek int64_t from network endian
	//require buf->readableBytes() >= sizeof(int64_t)
	int64_t peekInt64() const
	{
		assert(readableBytes() >= sizeof(int64_t));
		int64_t be64 = 0;
		::memcpy(&be64, peek(), sizeof be64);
		return sockets::networkToHost64(be64);
	}
	int32_t peekInt32() const
	{
		assert(readableBytes() >= sizeof(int32_t));
		int32_t be32 = 0;
		::memcpy(&be32, peek(), sizeof be32);
		return sockets::networkToHost32(be32);
	}
	int16_t peekInt16() const
	{
		assert(readableBytes() >= sizeof(int16_t));
		int32_t be16 = 0;
		::memcpy(&be16, peek(), sizeof be16);
		return sockets::networkToHost16(be16);
	}
	int8_t peekInt8() const
	{
		assert(readableBytes() >= sizeof(int8_t));
		int8_t be8 = *peek();
		return be8;
	}

	void prependInt64(int64_t x)
	{
		int64_t be64 = sockets::hostToNetwork64(x);
		prepend(&be64, sizeof be64);
	}

	void prependInt32(int32_t x)
	{
		int32_t be32 = sockets::hostToNetwork32(x);
		prepend(&be32, sizeof be32);
	}

	void prependInt16(int16_t x)
	{
		int16_t be16 = sockets::hostToNetwork16(x);
		prepend(&be16, sizeof be16);
	}
	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		m_readerIndex -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + m_readerIndex);
	}

	void shrink(size_t reserve)
	{
		Buffer other;
		other.ensureWritableBytes(readableBytes() + reserve);
		other.append(toStringPiece());
		swap(other);
		//m_buffers.shrink_to_fit();// don't cause the data loss
	}

	size_t internalCapacity() const 
	{
		return m_buffers.capacity();
	}

	// read data directly into buffer
	// it may implement with readv
	// @return result of read(), @c errno is saved
	ssize_t readFd(int fd, int* saveErrno);
private:
	char* begin()
	{
		return &*m_buffers.begin();
	}

	const char* begin() const
	{
		return &*m_buffers.begin();
	}

	void makeSpace(size_t len)
	{
		if( writableBytes() + prependableBytes() < len + kCheapPrepend )
		{
			m_buffers.resize(m_writerIndex + len);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(kCheapPrepend < m_readerIndex);
			size_t readable = readableBytes();
			std::copy(begin() + m_readerIndex, begin() + m_writerIndex, begin() + kCheapPrepend);
			m_readerIndex = kCheapPrepend;
			m_writerIndex = m_readerIndex + readable;
			assert(readable == readableBytes());
		}
	}

private:
	std::vector<char> m_buffers;
	size_t m_readerIndex;
	size_t m_writerIndex;

	static const char kCRLF[];

};


}

}

#endif

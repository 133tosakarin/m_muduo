#ifndef DC_BASE_STRINGPIECE_H
#define DC_BASE_STRINGPIECE_H

#include <cstring>
#include <iosfwd>
#include "types.h"

namespace dc
{

// for passing c-style string argument to a function.
class StringArg
{
public:
	StringArg(const char* str) : m_str(str) {}

	StringArg(const std::string& str) : m_str(str.c_str()) {}

	const char* c_str() const { return m_str; }

private:
	const char* m_str;
};

class StringPiece
{
private:
	const char* m_ptr;
	int m_length;
public:

	StringPiece() : m_ptr( nullptr ), m_length( 0 ) {}
	StringPiece( const char* str ) : m_ptr( str ), m_length( static_cast<int>(strlen(m_ptr))) {}

	StringPiece(const unsigned char* str ) : m_ptr(reinterpret_cast<const char*>(str)), m_length(static_cast<int>(strlen(m_ptr))) {}

	StringPiece(const std::string& str ) : m_ptr(str.data()), m_length( static_cast<int>(str.size())) {}

	StringPiece( const char* offset, int len ) : m_ptr( offset ), m_length( len ) {}

	const char* data() const { return m_ptr; }
	int size() const { return m_length; }
	bool empty() const { return m_length == 0; }
	const char* begin() const { return m_ptr; }
	const char* end() const { return m_ptr + m_length; }
	void clear() { m_ptr = nullptr; m_length = 0; }
	void set(const char* buffer, int len )
	{
		m_ptr = buffer;
		m_length = len;
	}

	void set(const char* str)
	{
		m_ptr = str;
		m_length = static_cast<int>(strlen(str));
	}

	void set( const void * buffer, int len )
	{
		m_ptr = reinterpret_cast<const char*>(buffer);
		m_length = len;
	}

	//I think operator[] should be returned;
	char operator[](int i) const { return m_ptr[i]; }

	void remove_prefix(int n) 
	{
		m_ptr +=n;
		m_length -=n;
	}

	void remove_suffix( int n )
	{
		m_length -=n;
	}

	bool operator==(const StringPiece& x) const 
	{
		//m_ptr is the same as x.m_ptr , that is their address is equal
		return ((m_length == x.m_length) && (memcmp(m_ptr, x.m_ptr, m_length) == 0 ) );
	}

	bool operator!=(const StringPiece& x) const 
	{
		return !(*this == x );
	}
	
	#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp) \
		bool operator cmp(const StringPiece& x) const { \
		int r = memcmp(m_ptr, x.m_ptr, m_length < x.m_length ? m_length : x.m_length ); \
		return ((r auxcmp 0) || ((r==0) && (m_length cmp x.m_length))); \
	}

	STRINGPIECE_BINARY_PREDICATE(<,<);
	STRINGPIECE_BINARY_PREDICATE(<=,<);
	STRINGPIECE_BINARY_PREDICATE(>=,>);
	STRINGPIECE_BINARY_PREDICATE(>,>);
	#undef STRINGPIECE_BINARY_PREDICATE
	
	int compare(const StringPiece& x) const 
	{
		int r = memcmp(m_ptr, x.m_ptr, m_length < x.m_length ? m_length : x.m_length );
		if( r==0 )
		{
			if( m_length < x.m_length ) r = -1;
			else if( m_length > x.m_length ) r = 1;
		}
		return r;
	}

	std::string as_string() const
	{
		return std::string(data(), size());
	}

	void copyToString(std::string* target) const
	{
		target->assign(m_ptr, m_length);
	}

	bool starts_with( const StringPiece& x) const
	{
		return ((m_length >= x.m_length ) && (memcmp(m_ptr, x.m_ptr,x.m_length) == 0));
	}



};

#ifdef HAVE_TYPE_TRAITS
// This makes vector<StriingPiece> really fast for some stl implementations
template<> struct __type_traits<dc::StringPieces>
{
	using has_trivial_default_constructor = __true_type;
	using has_trivial_copy_constructor = __true_type;
	using has_trivial_assignment_operator = __true_type;
	using has_trivial_destructor = __true_type;
	using is_POD_type = __true_type;
};
#endif
// allow StringPiece to be logged
std::ostream& operator<<(std::ostream& o, const dc::StringPiece& piece);
}
#endif

#ifndef DC_BASE_TYPES_H
#define DC_BASE_TYPES_H

#include <stdint.h>
#include <cstring>
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif
namespace dc
{
using std::string;
inline void memZero( void * p, size_t n )
{
	memset(p, 0, n);
}


template<typename To, typename From>
inline To implicit_cast(From const& f)
{
	return f;
}

template<typename To, typename From>
inline To down_cast(From* f)
{
	if(false)
	{
		implicit_cast<From*, To>(0);
	}
	#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUG_NO_RTTI)
		assert(f == nullptr || dynamic_cast<To>(f) != nullptr);
	#endif
	return static_cast<To>(f);
}


}
#endif

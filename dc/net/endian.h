#ifndef DC_NET_ENDIAN_H
#define DC_NET_ENDIAN_H

#include <stdint.h>
#include <endian.h>

namespace dc
{
namespace net
{
namespace sockets
{

// the inline assembler code make type blur,
// so we disable warning for a while.
#pragma GCC dignostic push
#pragma GCC dignostic ignored "-Wconversion"
#pragma GCC dignostic ignored "-Wold-style-cast"

inline uint64_t hostToNetwork64(uint64_t host64)
{
	return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
	return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
	return htobe16(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
	return betoh64(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
	return betoh32(net32);
}

inline uint16_t hostToNetwork16(uint16_t net16)
{
	return betoh16(net16);
}

#pragma GCC dianostic pop

} //namespace sockets



}//namespace net


}//namespace dc



#endif

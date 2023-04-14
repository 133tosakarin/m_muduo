#include "logStream.h"
#include <algorithm>
#include <stdint.h>
#include <limits>
#include <type_traits>
#include <cstring>
#include <cstdio>
#include <inttypes.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif 

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

namespace dc
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");
const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");
//Efficient Integer to string Conversions, by Matthew Wilson.
template<typename T>
size_t convert(char buf[], T value)
{
    T i = value;
    char* p = buf;
    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
        /* code */
    } while (i!=0);
    
    if(value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

size_t convertHex(char buf[], uintptr_t value )
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    }while( i != 0 );

    *p = '\0';
    std::reverse(buf, p );
    return p - buf;
    
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

/*
Format a number with 5 characters, includeing SI units.
[0, 999]
[1.00k, 999k]
[1.00m, 999m]

*/
std::string formatSI(int64_t s)
{
    double n = static_cast<double>(s);
    char buf[64];
    if( s < 1000)
        snprintf(buf, sizeof(buf), "%" PRId64, s);
    else if(s < 9995)
        snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
    else if (s < 99950)
      snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
    else if (s < 999500)
      snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
    else if (s < 9995000)
      snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
    else if (s < 99950000)
      snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
    else if (s < 999500000)
      snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
    else if (s < 9995000000)
      snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
    else if (s < 99950000000)
      snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
    else if (s < 999500000000)
      snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
    else if (s < 9995000000000)
      snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
    else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
    else if (s < 9995000000000000)
      snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
    else if (s < 99950000000000000)
      snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
    else if (s < 999500000000000000)
      snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
    else
      snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
    return buf;
}

std::string formatIEC(int64_t s)
{ 
  double n = static_cast<double>(s);
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;
  
  if (n < Ki)
    snprintf(buf, sizeof buf, "%" PRId64, s);
  else if (n < Ki*9.995) 
    snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
  else if (n < Ki*99.95) 
    snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
  else if (n < Ki*1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", n / Ki);
  
  else if (n < Mi*9.995) 
    snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
  else if (n < Mi*99.95) 
    snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
  else if (n < Mi*1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", n / Mi);
  
  else if (n < Gi*9.995) 
    snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
  else if (n < Gi*99.95) 
    snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
  else if (n < Gi*1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", n / Gi);
  
  else if (n < Ti*9.995) 
    snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
  else if (n < Ti*99.95) 
    snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
  else if (n < Ti*1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", n / Ti);
  
  else if (n < Pi*9.995) 
    snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
  else if (n < Pi*99.95) 
    snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
  else if (n < Pi*1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", n / Pi);
  
  else if (n < Ei*9.995) 
    snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
  else
    snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
  return buf;
}

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
  *m_cur = '\0';
  return m_data;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart()
{

}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd()
{

}

void LogStream::staticCheck()
{
    static_assert(kMaxNumbericSize - 10 > std::numeric_limits<double>::digits10,
                    "kMaxNumericSize is large enough");
    static_assert(kMaxNumbericSize - 10 > std::numeric_limits<long double>::digits10,
                    "kMaxNumericSize is large enough");
    static_assert(kMaxNumbericSize - 10 > std::numeric_limits<long>::digits10);

    static_assert(kMaxNumbericSize - 10 > std::numeric_limits<long long>::digits10,
                    "kMaxNumericSize is large enough");
}

template<typename T>
void LogStream::formatInteger(T v)
{
    if( m_buffer.avail() >= kMaxNumbericSize)
    {
        size_t len = convert(m_buffer.current(), v);
        m_buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v )
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v )
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v )
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long  long v )
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v )
{
    formatInteger(v);
    return *this;
}
// potraying a pointer uses Hex
LogStream& LogStream::operator<<(const void * p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if(m_buffer.avail() >= kMaxNumbericSize)
    {
        char* buf = m_buffer.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t  len = convertHex(buf + 2, v);
        m_buffer.add(len + 2);
    }
    return *this;
}
//replace this with Grisu3 by Florian Loitsch
LogStream& LogStream::operator<<(double v)
{
    if(m_buffer.avail() >= kMaxNumbericSize)
    {
      int len = snprintf(m_buffer.current(), kMaxNumbericSize, "%.12g", v);
      m_buffer.add(len);
    }
    return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val )
{
  static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
  m_length = snprintf(m_buf, sizeof(m_buf), fmt, val);
  assert(static_cast<size_t>(m_length) < sizeof m_buf);
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);

}

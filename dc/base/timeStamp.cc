
#include"timeStamp.h"
#include<sys/time.h>
#include<cstdio>
#include<iostream>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif



namespace dc
{
static_assert( sizeof(Timestamp) == sizeof(int64_t ),
				"Timestamp should be same size as int64_t");

std::string Timestamp::toString() const
{
	char buf[32] = {0};
	int64_t seconds = m_microSecondsSinceEpoch / kMicroSecondsPerSecond;

	int64_t microseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;
	snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds );
	return buf;

}

std::string Timestamp::toFormattedString(bool showMicroseconds ) const
{
	char buf[64] = {0};
	time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond );
	struct tm tm_time;
	gmtime_r(&seconds, &tm_time );

	if( showMicroseconds)
	{
		int microseconds = static_cast< int > ( m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);
	
	}
	else
	{
		//int microseconds = static_cast< int > ( m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	}
	return buf;
}

Timestamp Timestamp::now()
{

	struct timeval tv;
	gettimeofday(&tv, nullptr );
	int64_t seconds = tv.tv_sec;
	return Timestamp( seconds * kMicroSecondsPerSecond + tv.tv_usec );
}
}

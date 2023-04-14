#ifndef DC_TIMEZONE_H
#define DC_TIMEZONE_H
#include<time.h>
#include<memory>
#include "noncopyable.h"
#include "date.h"
namespace dc
{

//Local time in unspecifed timezone.
//A minute is always 0 seconds, no leap second.


struct DateTime
{
	DateTime() {}
	explicit DateTime(const struct tm&);
	DateTime( int year, int month, int day, int hour, int minute, int second )
		: m_year( year ), m_month( month ), m_day( day ), m_hour( hour ), m_minute( minute ), m_second( second ) {}

	//"2011-12-31 12:31:56"
	std::string toIsoString() const;
	int m_year = 0;
	int m_month = 0;
	int m_day = 0;
	int m_hour = 0;
	int m_minute = 0;
	int m_second = 0;

};

//TimeZone for 1970-2100
class TimeZone
{
public:
	TimeZone() = default; // an invalid timezone
	TimeZone( int eastOfUtc, const char* tzame ); // a fixed timezone
	static TimeZone UTC();
	static TimeZone China();
	static TimeZone loadZoneFile( const char* zonefile );
	
	bool valid() const
	{
		// explicit operator bool() const 
		return static_cast< bool > (m_data );
	}
	//utc时间转换成当地时间
	struct DateTime toLocalTime(int64_t secondsSinceEpoch, int* utcOffset = nullptr ) const;
	//当地时间转换成utc时间
	int64_t fromLocalTime( const struct DateTime&, bool postTransition = false )const;
	//gmtime, 当地时间转换成utc时间
	static struct DateTime toUtcTime( int64_t secondsSinceEpoch );
	//timegm，转换成当地找时间
	static int64_t fromUtcTime(const struct DateTime& );
	
	struct Data;


private:
	explicit TimeZone( std::unique_ptr< Data > data );
	std::shared_ptr< Data > m_data;


};

}
#endif

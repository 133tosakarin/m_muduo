#include"timeZone.h"
#include<algorithm>
#include<endian.h>  //The sorting of bytes restoring
#include<stdexcept>
#include<string>
#include<vector>
#include<assert.h>

namespace dc
{

struct TimeZone::Data
{
	//记录utc时间和本地时间的转化信息的基本单元
	struct Transition
	{
		int64_t utctime; //utc时间
		int64_t localtime; //本地时间
		int localtimeIdx; //data.localtimes是对应的本地时间的索引

		Transition(int64_t t, int64_t l, int localIdx )
			: utctime(t), localtime( l ), localtimeIdx( localIdx ) {}

	};

	//记录本地时间信息转化单元
	struct LocalTime
	{
		int32_t utcOffset; //east of utc， utc转化为本地时间的差值
		bool isDst;		//夏令时标记
		int desigIdx;	//data.names对应的索引

		LocalTime( int32_t offset, bool dst, int idx ) : utcOffset( offset ), isDst( dst), desigIdx( idx ) {}

	};


	void addLocalTime( int32_t utcOffset, bool isDst, int desigIdx )
	{
		localtimes.push_back( LocalTime( utcOffset, isDst, desigIdx ) );		
	}

	void addTransition( int64_t utcTime, int localIdx )
	{
		LocalTime lt = localtimes.at( localIdx );
		transitions.push_back( Transition( utcTime, utcTime + lt.utcOffset, localIdx ) );
	}

	const LocalTime* findLocalTime( int64_t utctime ) const;
	const LocalTime* findLocalTime( const struct DateTime& local, bool postTransition ) const;

	struct CompareUtcTime
	{
		bool operator()( const Transition& lhs, const Transition& rhs ) const
		{
			return lhs.utctime < rhs.utctime;
		}
	};

	struct CompareLocalTime
	{
		bool operator()( const Transition& lhs, const Transition& rhs ) const 
		{
			return lhs.localtime < rhs.localtime;
		}
	};

	std::vector<Transition> transitions;
	std::vector<LocalTime> localtimes;
	std::string abbrevition;
	std::string tzstring;

};

const int kSecondsPerDay = 24 * 60 * 60;

//提取文件读取工具函数，为方便取自定字节的文本信息，主要用于读取特定格式的时区文本信息
class File : noncopyable
{
public:
	File(const char* file) : m_fp(fopen(file,"rb"))
	{

	} 

	~File()
	{
		if(m_fp)
		{
			fclose(m_fp);
		}
	}

	bool valid() const { return m_fp; }

	std::string readBytes( int n)
	{
		char buf[n];
		ssize_t nr = fread(buf, 1, n, m_fp);
		if( nr != n)
			throw std::logic_error("no enough data");
		return std::string(buf, n);
	}

	std::string readToEnd()
	{
		char buf[4096];
		ssize_t nr = 0;
		std::string res;
		while( (nr = ::fread(buf, 1, sizeof(buf),m_fp )) > 0)
		{
			res.append(buf, nr);
		}
		return res;
		
	}

	int64_t readInt64()
	{
		int64_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(int64_t), m_fp);
		if( nr != sizeof(int64_t))
			throw std::logic_error("bad int64_t data");
		return be64toh(x);
	}

	int32_t readInt32()
	{
		int32_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(int32_t), m_fp);
		if( nr != sizeof(int32_t) )
			throw std::logic_error("bad int64_t data");
		return be32toh(x);
	}

	int32_t readUint8()
	{
		uint8_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(uint8_t), m_fp);
		if( nr != sizeof(uint8_t) )
			throw std::logic_error("bad uint8_t data");
		return x;
	}

	off_t skip(ssize_t bytes)
	{
		return ::fseek(m_fp, bytes, SEEK_CUR );
	}
private:
	FILE * m_fp;
};


bool readDataBlock(File& f, struct TimeZone::Data* data, bool v1)
{
	const int time_size = v1 ? sizeof(int32_t) : sizeof(int64_t);
	const int32_t isUtcCnt = f.readInt32();
	const int32_t isStdCnt = f.readInt32();
	const int32_t leapCnt = f.readInt32();
	const int32_t timeCnt = f.readInt32();
	const int32_t typeCnt = f.readInt32();
	const int32_t charCnt = f.readInt32();

	if(leapCnt != 0)
		return false;
	if( isUtcCnt != 0 && isUtcCnt != typeCnt )
		return false;
	if(isStdCnt != 0 && isStdCnt != typeCnt)
		return false;
	std::vector<int64_t> trans;
	trans.reserve(timeCnt);
	for( int i = 0; i < timeCnt; ++i)
	{
		if( v1 )
			trans.push_back(f.readInt32());
		else
			trans.push_back(f.readInt64());
	}

	std::vector<int> localtimes;
	localtimes.reserve(timeCnt);
	for( int i = 0; i < timeCnt; ++i )
	{
		uint8_t local = f.readUint8();
		localtimes.push_back(local);
	}

	data->localtimes.reserve(typeCnt);

	for( int i = 0; i < typeCnt; ++i )
	{
		int32_t gmtoff = f.readInt32();
		uint8_t isdst = f.readUint8();
		uint8_t abbrind = f.readUint8();
		data->addLocalTime(gmtoff, isdst, abbrind);
	}

	for( int i = 0; i < timeCnt; ++i )
	{
		int localIdx = localtimes[i];
		data->addTransition(trans[i], localIdx);
	}

	data->abbrevition = f.readBytes(charCnt);
	f.skip(leapCnt * (time_size + 4));
	f.skip(isStdCnt);
	f.skip(isUtcCnt);
	if(!v1)
	{
		data->tzstring = f.readToEnd();
	}
	return true;
}

bool readTimeZoneFile(const char* zoneFile, struct TimeZone::Data* data)
{
	File f(zoneFile);
	if(f.valid())
	{
		try
		{
			std::string head = f.readBytes(4);
			if(head != "TZlf")
				throw std::logic_error("bad head");
			std::string version = f.readBytes(1);
			f.readBytes(15);

			const int32_t isgmtCnt = f.readInt32();
			const int32_t isStdCnt = f.readInt32();
			const int32_t leapCnt = f.readInt32();
			const int32_t timeCnt = f.readInt32();
			const int32_t typeCnt = f.readInt32();
			const int32_t charCnt = f.readInt32();
			
			if( version =="2")
			{
				size_t skip = sizeof(int32_t) * timeCnt + timeCnt + 6 * typeCnt
								+ charCnt + 8 * leapCnt + isStdCnt + isgmtCnt;
				f.skip(skip);
				head = f.readBytes(4);
				if( head != "TZlf")
					throw std::logic_error("bad head");
				f.skip(16);
				return readDataBlock(f, data, false);
			}
			else
			{
				f.skip(-4 * 6);
				return readDataBlock(f, data, true);
			}
		}
		catch(const std::logic_error& e)
		{
			fprintf(stderr, "%s\n", e.what());
		}
		
	}
	return false;
}

inline void fillHMS(unsigned seconds, struct DateTime* dt)
{
	dt->m_second = seconds %60;
	unsigned minutes = seconds / 60;
	dt->m_minute = minutes %60;
	dt->m_hour = minutes/60;
}

DateTime BreakTime(int64_t t)
{
	struct DateTime dt;
	int seconds = static_cast<int>(t % kSecondsPerDay);
	int days = static_cast<int>(t / kSecondsPerDay );
	if( seconds < 0 )
	{
		seconds +=kSecondsPerDay;
		--days;
	}
	fillHMS(seconds, &dt);
	Date date(days + Date::kJulianDayOf1970_01_01);
	Date::YearMonthDay ymd = date.yearMonthDay();
	dt.m_year = ymd.year;
	dt.m_month = ymd.month;
	dt.m_day = ymd.day;
	return dt;
}

const TimeZone::Data::LocalTime* TimeZone::Data::findLocalTime(int64_t utcTime) const
{
	const LocalTime* local = nullptr;
	if(transitions.empty() || utcTime < transitions.front().utctime)
	{
		local = &localtimes.front();
	}
	else
	{
		Transition sentry(utcTime, 0, 0);
		auto transIt = std::upper_bound(transitions.begin(), transitions.end(), sentry, CompareUtcTime());
		assert(transIt != transitions.begin());
		if(transIt != transitions.end())
		{
			--transIt;
			local = &localtimes[transIt->localtimeIdx];
		}
		else
		{
			local = &localtimes[transitions.back().localtimeIdx];
		}
	}
	return local;
}

const TimeZone::Data::LocalTime* TimeZone::Data::findLocalTime(
		const struct DateTime& lt, bool postTransition) const
{
	const int64_t localtime = fromUtcTime(lt);
	if(transitions.empty() || localtime < transitions.front().localtime)
	{
		return &localtimes.front();
	}

	Transition sentry(0, localtime, 0);
	auto transIt = std::upper_bound(transitions.begin(), transitions.end(), sentry, CompareLocalTime());
	assert(transIt == transitions.begin());
	if (transIt == transitions.end())
    {
      // FIXME: use TZ-env
      return &localtimes[transitions.back().localtimeIdx];
    }
  
    Transition prior_trans = *(transIt - 1);
    int64_t prior_second = transIt->utctime - 1 + localtimes[prior_trans.localtimeIdx].utcOffset;
  
    // row UTC time             isdst  offset  Local time (PRC)     Prior second local time
    //  1  1989-09-16 17:00:00Z   0      8.0   1989-09-17 01:00:00
    //  2  1990-04-14 18:00:00Z   1      9.0   1990-04-15 03:00:00  1990-04-15 01:59:59
    //  3  1990-09-15 17:00:00Z   0      8.0   1990-09-16 01:00:00  1990-09-16 01:59:59
    //  4  1991-04-13 18:00:00Z   1      9.0   1991-04-14 03:00:00  1991-04-14 01:59:59
    //  5  1991-09-14 17:00:00Z   0      8.0   1991-09-15 01:00:00
  
    // input 1991-04-14 02:30:00, found row 4,
    //  4  1991-04-13 18:00:00Z   1      9.0   1991-04-14 03:00:00  1991-04-14 01:59:59
    if (prior_second < localtime)
    {
      // it's a skip
      // printf("SKIP: prev %ld local %ld start %ld\n", prior_second, localtime, transI->localtime);
      if (postTransition)
      {
        return &localtimes[transIt->localtimeIdx];
      }
      else
      {
        return &localtimes[prior_trans.localtimeIdx];
      }
    }
  
    // input 1990-09-16 01:30:00, found row 4, looking at row 3
    //  3  1990-09-15 17:00:00Z   0      8.0   1990-09-16 01:00:00  1990-09-16 01:59:59
    --transIt;
    if (transIt != transitions.begin())
    {
      prior_trans = *(transIt - 1);
      prior_second = transIt->utctime - 1 + localtimes[prior_trans.localtimeIdx].utcOffset;
    }
    if (localtime <= prior_second)
    {
      // it's repeat
      // printf("REPEAT: prev %ld local %ld start %ld\n", prior_second, localtime, transI->localtime);
      if (postTransition)
      {
        return &localtimes[transIt->localtimeIdx];
      }
      else
      {
        return &localtimes[prior_trans.localtimeIdx];
      }
    }
  
    // otherwise, it's unique
    return &localtimes[transIt->localtimeIdx];
}

// static
 TimeZone TimeZone::UTC()
 {
   return TimeZone(0, "UTC");
 }
 
 // static
 TimeZone TimeZone::loadZoneFile(const char* zonefile)
 {
   std::unique_ptr<Data> data(new Data);
   if (!readTimeZoneFile(zonefile, data.get()))
   {
     data.reset();
   }
   return TimeZone(std::move(data));
 }
 
 TimeZone::TimeZone(std::unique_ptr<Data> data)
   : m_data(std::move(data))
 {
 }
 
 TimeZone::TimeZone(int eastOfUtc, const char* name)
   : m_data(new TimeZone::Data)
 {
   m_data->addLocalTime(eastOfUtc, false, 0);
   m_data->abbrevition = name;
 }
 
 struct DateTime TimeZone::toLocalTime(int64_t seconds, int* utcOffset) const
 {
   struct DateTime localTime;
   assert(m_data != NULL);
 
   const Data::LocalTime* local = m_data->findLocalTime(seconds);
 
   if (local)
   {
     localTime = BreakTime(seconds + local->utcOffset);
     if (utcOffset)
     {
       *utcOffset = local->utcOffset;
     }
   }
 
   return localTime;
 }
 
 int64_t TimeZone::fromLocalTime(const struct DateTime& localtime, bool postTransition) const
 {
   assert(m_data != NULL);
   const Data::LocalTime* local = m_data->findLocalTime(localtime, postTransition);
   const int64_t localSeconds = fromUtcTime(localtime);
   if (local)
   {
     return localSeconds - local->utcOffset;
   }
   // fallback as if it's UTC time.
   return localSeconds;
 }
 
 DateTime TimeZone::toUtcTime(int64_t secondsSinceEpoch)
 {
   return BreakTime(secondsSinceEpoch);
 }
 
 int64_t TimeZone::fromUtcTime(const DateTime& dt)
 {
   Date date(dt.m_year, dt.m_month, dt.m_day);
   int secondsInDay = dt.m_hour * 3600 + dt.m_minute * 60 + dt.m_second;
   int64_t days = date.julianDayNumber() - Date::kJulianDayOf1970_01_01;
   return days * kSecondsPerDay + secondsInDay;
 }
 
 
 DateTime::DateTime(const struct tm& t)
   : m_year(t.tm_year + 1900), m_month(t.tm_mon + 1), m_day(t.tm_mday),
     m_hour(t.tm_hour), m_minute(t.tm_min), m_second(t.tm_sec)
 {
 }
 
 string DateTime::toIsoString() const
 {
   char buf[64];
   snprintf(buf, sizeof buf, "%04d-%02d-%02d %02d:%02d:%02d",
            m_year, m_month, m_day, m_hour, m_minute, m_second);
   return buf;
 }

}

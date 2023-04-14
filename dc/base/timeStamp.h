#ifndef DC_BASE_TIMESTAMP_H
#define DC_BASE_TIMESTAMP_H
#include<inttypes.h>
#include<boost/operators.hpp>

namespace dc
{

class Timestamp : public boost::equality_comparable1< Timestamp >,
				  public boost::less_than_comparable< Timestamp >
{

public:
	//constructg an invalid Timestamp
	Timestamp() : m_microSecondsSinceEpoch( 0 ) {}
	//construct a Timestamp at specific time
	explicit Timestamp( int64_t microSecondsSinceEpoch ) : m_microSecondsSinceEpoch( microSecondsSinceEpoch ){}
	
	void swap(Timestamp& that )
	{
		std::swap(m_microSecondsSinceEpoch, that.m_microSecondsSinceEpoch);
	}

	//default copy/assignment/dtor are okay
	
	std::string toString() const;
	std::string toFormattedString( bool showMicroseconds = true ) const;

	bool valid() const { return m_microSecondsSinceEpoch > 0;} 

	int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch;}

	time_t secondsSinceEpoch() const 
	{
		return static_cast< time_t > (m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
	}

	//get time of now
	
	static Timestamp now();
	static Timestamp invalid() { return Timestamp();}

	static Timestamp fromUnixTime( time_t t )
	{
		return fromUnixTime( t, 0 );
	}

	static Timestamp fromUnixTime( time_t t, int microseconds )
	{
		return Timestamp( static_cast< int64_t >(t) * kMicroSecondsPerSecond + microseconds );
	}

	static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
	int64_t m_microSecondsSinceEpoch;
};

inline bool operator<(Timestamp lhs, Timestamp rhs )
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
//gets time difference of two timestamps, result in seconds
inline double timeDifference( Timestamp high, Timestamp low )
{
	int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

//add seconds to given timestamp
inline Timestamp addTime( Timestamp timestamp, double seconds )
{
	int64_t delta = static_cast< int64_t >( seconds* Timestamp::kMicroSecondsPerSecond );
	return Timestamp( timestamp.microSecondsSinceEpoch() + delta);
}
}

#endif

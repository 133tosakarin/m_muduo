/*
 * =====================================================================================
 *
 *       Filename:  test_timestamp.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/05/2023 10:08:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "../dc/base/timeStamp.h"
#include <vector>
void passbyvalue(dc::Timestamp v)
{
	printf("time is: %s", v.toString().c_str());
}

void passbyreference(dc::Timestamp& v)
{
	printf("time is: %s", v.toString().c_str());
}
void bench()
{
	const int kBench = 1000 * 1000;
	std::vector<dc::Timestamp> vec;
	vec.reserve(kBench);
	for( int i = 0; i < kBench; ++i )
	{
		vec.push_back(dc::Timestamp::now());
	}
	printf("%s\n", vec.front().toString().c_str());
	printf("%s\n", vec.back().toString().c_str());
	printf("%f\n", dc::timeDifference(vec.front(), vec.back()));
	int increments[100] = {0};
	int64_t start = vec.front().microSecondsSinceEpoch();
	for( int i = 1;  i< kBench; ++i )
	{
		int64_t next = vec[i].microSecondsSinceEpoch();
		int64_t inc = next - start;
		if( inc < 0 )
		{
			printf("reverse!\n");
		}
		else if( inc < 100 )
		{
			++increments[inc];
		}
		else
		{
			printf("big gap %d\n", static_cast<int>(inc));
		}
		start = next;
	}
	for( int i = 0;  i< 100; ++i )
	{
		printf("%2d: %d\n", i, increments[i]);
	}
}

	int
main ( int argc, char *argv[] )
{
	dc::Timestamp start(dc::Timestamp::now());
	printf("now is : %s\n", start.toString().c_str());
	passbyvalue(start);
	passbyreference(start);
	bench();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

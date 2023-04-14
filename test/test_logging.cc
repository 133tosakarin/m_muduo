#include "logging.h"
#include "threadpool.h"
#include "logFile.h"
#include "timeZone.h"

#include <cstdio>
#include <unistd.h>


#include	<stdlib.h>

long long g_total;
FILE * g_file;
std::unique_ptr<dc::LogFile> g_logFile;

void dummyOutput(const char* msg, int len)
{
	g_total +=len;
	if(g_file)
	{
		fwrite(msg, 1, len, g_file);
	}
	else if(g_logFile)
	{
		g_logFile->append(msg, len);	
	}
}
void bench(const char* type, bool is_long = false)
{
	dc::Logger::setOutput(dummyOutput);
	dc::Timestamp start(dc::Timestamp::now());
	g_total = 0;
	int n = 1000  *1000;
	const bool kLongLog = is_long;
	dc::string empty = " ";
	dc::string longStr(3000, 'X');
	longStr +=" ";
	for( int i = 0; i < n; ++i )
	{
		LOG_INFO << "Hello 0123456780" << " abcdefghijklmnopqrstuvwxyz"
				 << (kLongLog ? longStr : empty)
				 << i;
	}
	dc::Timestamp end(dc::Timestamp::now());
	double seconds = dc::timeDifference(end, start);
	printf("%12s: %lf seconds, %lld bytes, %10.2lf msg/s, %.2lf MiB/s\n",
			type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));

}

void logInThread()
{
	LOG_INFO << "loginThread";
	usleep(1000);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
	int
main ( int argc, char *argv[] )
{
	bool is_long = argc > 1;
	getpid(); // for Itrace ans strace
	dc::ThreadPool pool("pool");
	pool.start(5);
	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);
	
	LOG_TRACE << "trace";
	LOG_DEBUG << "debug";
	LOG_INFO <<  "hello";
	LOG_WARN << "world";
	LOG_ERROR << "Error";
	LOG_INFO << sizeof(dc::Logger);
	LOG_INFO << sizeof(dc::LogStream);
	LOG_INFO << sizeof(dc::Fmt);
	LOG_INFO << sizeof(dc::LogStream::Buffer);
	sleep(1);
	bench("nop");
	char buffer[64 * 1024];
	g_file = fopen("/dev/null", "w");
	setbuffer(g_file, buffer, sizeof buffer);
	bench("/dev/null");
	fclose(g_file);

	g_file = fopen("/tmp/log", "w");
	setbuffer(g_file, buffer, sizeof buffer);
	bench("/tmp/log");
	fclose(g_file);

	g_file = nullptr;
	g_logFile.reset(new dc::LogFile("test_log_st thread no safe", 500 * 1000 * 1000, false));
	bench("test_log_st", is_long);

	g_logFile.reset(new dc::LogFile("test_log_mt thread safe", 500 * 1000 * 1000, true ));
	bench("test_log_mt", is_long);
	g_logFile.reset();
	{
		g_file = stdout;
		sleep(1);
		dc::TimeZone beijing(8 * 3600, "CST");
		dc::Logger::setTimeZone(beijing);
		LOG_TRACE << "trace CST";
		LOG_DEBUG << "debug CST";
		LOG_INFO << "Hello CST";
		LOG_WARN << "World CST";
		LOG_ERROR << "Error CST";

		sleep(1);
		dc::TimeZone newyork = dc::TimeZone::loadZoneFile("/usr/share/zoneinfo/America/New_York");
		dc::Logger::setTimeZone(newyork);

		LOG_TRACE << "trace CST";
		LOG_DEBUG << "debug CST";
		LOG_INFO << "Hello CST";
		LOG_WARN << "World CST";
		LOG_ERROR << "Error CST";
		g_file = nullptr;

	}
	bench("timezone nop");
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

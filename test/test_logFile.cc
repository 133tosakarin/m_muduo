#include "logFile.h"
#include "logging.h"

#include <unistd.h>


#include	<stdlib.h>
std::unique_ptr<dc::LogFile> g_logFile;
void outputFunc(const char* msg, int len)
{
	g_logFile->append(msg, len);
}

void flushFunc()
{
	g_logFile->flush();
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
	char name[256] = {'\0'};
	strncpy(name, argv[0], sizeof name - 1);
	g_logFile.reset(new dc::LogFile(::basename(name), 200 * 1000));
	dc::Logger::setOutput(outputFunc);
	dc::Logger::setFlush(flushFunc);
	dc::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	for( int i = 0; i < 10000; ++i )
	{
		LOG_INFO << line << i;
		usleep(1000);
	}


	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */



#include "../dc/base/logging.h"
#include "../dc/base/timeStamp.h"
#include "../dc/base/timeZone.h"
#include <iostream>
#include "../dc/base/thread.h"
void printTm(dc::DateTime& t)
{
    printf("%04d-%02d-%02d %02d:%02d:%02d \n",t.m_year, t.m_month, t.m_day, t.m_hour, t.m_minute, t.m_second);
}

int i = 0;
int main()
{
    
    //dc::Timestamp timestamp = dc::Timestamp::now();
    LOG_INFO << " test";
    
    //printTm(t);
    dc::Thread thread([](){
        auto t = dc::TimeZone::toUtcTime(dc::Timestamp::now().secondsSinceEpoch());
        printTm(t);
        for(  ; i <= 1000; ++i )
        LOG_INFO << " hello " + std::to_string(i);}, "dengchao");
    dc::Thread thread2([](){
        auto t = dc::TimeZone::toUtcTime(dc::Timestamp::now().secondsSinceEpoch());
        printTm(t);
        for(  ; i <= 1000; ++i ){
        LOG_INFO << " hi " + std::to_string(i);} }, "dengchao");

    thread2.start();

    thread.start();
    
    thread.join();
    thread2.join();
	printf("%ld\n", sizeof(int32_t));
	//LOG_DEBUG << "hello";
}

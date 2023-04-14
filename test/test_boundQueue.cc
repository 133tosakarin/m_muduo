#include <stdlib.h>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cstdio>
#include "countDownLatch.h"
#include "thread.h"
#include "logging.h"
#include "boundedBlockingQueue.h"
#include "countDownLatch.h"
class Test
{
public:
	Test(int numThreads) : m_queue(20),m_latch(numThreads)
	{
		for( int i = 0; i < numThreads; ++i )
		{
			char name[32];
			snprintf(name, sizeof name, "work thread%d", i );
			m_threads.emplace_back(new dc::Thread(std::bind(&Test::threadFunc, this),dc::string(name)));
		}
		for(auto& thr : m_threads)
		{
			thr->start();
		}
	}

	void run(int times)
	{
		printf("waiting for count down latch\n");
		m_latch.wait();
		printf("all threads started\n");
		for( int i = 0; i < times; ++i )
		{
			char buf[32];
			snprintf(buf, sizeof buf, "hello %d", i );
			m_queue.put(buf);
			printf("tid = %d, put data = %s, size = %zd\n", dc::CurrentThread::tid(), buf,m_queue.size());	
		}
	}

	void joinAll()
	{
		for( size_t i = 0; i < m_threads.size(); ++i )
		{
			m_queue.put("stop");
		}

		for( auto& thr : m_threads )
		{
			thr->join();
		}
	}
private:
	void threadFunc()
	{
		LOG_INFO << m_threads.size() << "started";
		m_latch.countDown();
		bool running = true;
		while(running)
		{
			dc::string v(m_queue.take());
			printf("tid=%d, get data = %s, size = %zd\n", dc::CurrentThread::tid(), v.c_str(), m_queue.size());
			running = (v != "stop");
			
		}
		printf("tid=%d, %s stopped\n", 
				dc::CurrentThread::tid(),
				dc::CurrentThread::name());
	}
	dc::BoundedBlockingQueue<dc::string> m_queue;
	dc::CountDownLatch m_latch;
	std::vector<std::unique_ptr<dc::Thread>> m_threads;
};
	int
main ( int argc, char *argv[] )
{
	printf("pid = %d, tid = %d, name = %s\n",
			::getpid(), dc::CurrentThread::tid(), dc::CurrentThread::name());
	Test t(5);
	t.run(100);
	t.joinAll();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


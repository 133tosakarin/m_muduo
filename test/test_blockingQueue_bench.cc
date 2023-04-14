/*
 * =====================================================================================
 *
 *       Filename:  test_blockingQueue_bench.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/07/2023 01:14:45 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "blockingQueue.h"
#include "countDownLatch.h"
#include "logging.h"
#include "timeStamp.h"
#include "thread.h"

#include <stdlib.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <unistd.h>

bool g_verbose = false;

class Bench
{
public:
	Bench(int numThreads) : m_latch(numThreads)
	{
		m_threads.reserve(numThreads);
		for( int i = 0; i < numThreads; ++i )
		{
			char name[32];
			snprintf(name, sizeof name, "work thread%d", i );
			m_threads.emplace_back(new dc::Thread(std::bind(&Bench::threadFunc, this), dc::string(name)));

		}
		for( auto& thr : m_threads)
		{
			thr->start();
		}
	}
	void run(int times)
	{
		printf("waiting for count down latch\n");
		m_latch.wait();
		LOG_INFO << m_threads.size() << " threads started";
		int64_t total_delay = 0;
		for( int i = 0; i < times; ++i )
		{
			dc::Timestamp now(dc::Timestamp::now());
			m_queue.put(now);
			total_delay += m_delay_queue.take();

		}
		printf("Average delay: %.4fus\n", static_cast<double>(total_delay)/times);
	}
	void joinAll()
	{
		for( size_t i = 0; i < m_threads.size(); ++i )
		{
			m_queue.put(dc::Timestamp::invalid());
		}
		for( auto& thr : m_threads )
		{
			thr->join();
		}
		LOG_INFO << m_threads.size() << " threads stopped";
	}
private:
	void threadFunc()
	{
		if( g_verbose)
		{
			printf("tid = %d, %s started\n",
					dc::CurrentThread::tid(),
					dc::CurrentThread::name());
		}
		std::map<int,int> delays;
		m_latch.countDown();
		bool running = true;
		while(running)
		{
			dc::Timestamp t(m_queue.take());
			dc::Timestamp now(dc::Timestamp::now());
			if(t.valid())
			{
				int delay = static_cast<int>(dc::timeDifference(now, t) * 1000000);
				++delays[delay];
				m_delay_queue.put(delay);
			}
			running = t.valid();
		}

		if( g_verbose )
		{
			printf("tid = %d, %s stopped\n",
					dc::CurrentThread::tid(),
					dc::CurrentThread::name());

			for(const auto& delay: delays)
			{
				printf("tid = %d, delay = %d, count = %d\n",
						dc::CurrentThread::tid(),
						delay.first, delay.second);
			}
		}
	}
	dc::BlockingQueue<dc::Timestamp> m_queue;
	dc::BlockingQueue<int> m_delay_queue;
	dc::CountDownLatch m_latch;
	std::vector<std::unique_ptr<dc::Thread>> m_threads;
};

	int
main ( int argc, char *argv[] )
{
	int threads = argc > 1 ? atoi(argv[1]) : 1;
	Bench t(threads);
	t.run(100000);
	t.joinAll();
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

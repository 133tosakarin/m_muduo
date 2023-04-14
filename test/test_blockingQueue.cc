/*
 * =====================================================================================
 *
 *       Filename:  blockingQueue_test.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/06/2023 11:24:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "blockingQueue.h"
#include "countDownLatch.h"
#include "thread.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>
class Test
{
	public:
		Test(int numThreads) : m_latch(numThreads)
	{
		for( int i = 0; i < numThreads; ++i )
		{
			char name[32];
			snprintf(name, sizeof name, "work thread%d", i);
			m_threads.emplace_back(new dc::Thread(std::bind(&Test::threadFunc, this), dc::string(name)));

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
			printf("all threads started\n");
			for( int i = 0; i < times; ++i)
			{
				char buf[32];
				snprintf(buf, sizeof buf, "hello %d", i );
				m_queue.put(buf);
				printf("tid = %d, put data = %s, size = %zd\n", dc::CurrentThread::tid(), buf, m_queue.size());
			}
		}
		void joinAll()
		{
			for (size_t i = 0; i < m_threads.size(); ++i)
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
			printf("tid = %d, %s started\n",
					dc::CurrentThread::tid(),
					dc::CurrentThread::name());
			m_latch.countDown();
			bool running = true;
			while(running)
			{
				std::string d(m_queue.take());
				printf("tid = %d, get data = %s, size = %zd\n", dc::CurrentThread::tid(), d.c_str(), m_queue.size());
				running = (d != "stop");
			}
			printf("tid = %d, %s stopped\n",
					dc::CurrentThread::tid(),
					dc::CurrentThread::name());
		}
		dc::BlockingQueue<std::string> m_queue;
		dc::CountDownLatch m_latch;
		std::vector<std::unique_ptr<dc::Thread>>m_threads;

};

void testMove()
{
	dc::BlockingQueue<std::unique_ptr<int>>queue;
	queue.put(std::unique_ptr<int>(new int(42)));
	std::unique_ptr<int> x = queue.take();
	printf("took %d\n", *x);
	*x = 123;
	queue.put(std::move(x));
	std::unique_ptr<int> y = queue.take();
	printf("took %d\n", *y);
}

	int
main ( int argc, char *argv[] )
{
	printf("pid = %d, tid=%d, name = %s\n", ::getpid(),
			dc::CurrentThread::tid(), dc::CurrentThread::name());
	Test t(5);
	t.run(100);
	t.joinAll();
	testMove();
	printf("number of created threads %d\n", dc::Thread::numCreated());
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

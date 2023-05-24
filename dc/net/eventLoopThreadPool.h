/*
 * =====================================================================================
 *
 *       Filename:  eventLoopThreadPool.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2023 11:55:40 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef DC_NET_EVENTLOOPTHREADPOOL_H
#define DC_NET_EVENTLOOPTHREADPOOL_H


#include "dc/base/noncopyable.h"
#include "dc/base/types.h"
#include <functional>
#include <memory>
#include <vector>


namespace dc
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop* )>;

	EventLoopThreadPool(EventLoop* loop, const string& nameArg);
	~EventLoopThreadPool();
	void setThreadNum(int numThreads) { m_numThreads = numThreads; }
	void start(const ThreadInitCallback& cb = ThreadInitCallback());

	// valid after calling start()
	//round-robin
	EventLoop* getNextLoop();

	// with the samme hash code , it will always return the same EventLoop
	EventLoop* getLoopForHash(size_t hashCode);

	std::vector<EventLoop*> getAllLoops();

	bool started() const { return is_started; }
	const string& name() const { return m_name; } 
private:
	EventLoop* m_baseLoop;
	string m_name;
	bool is_started;
	int m_numThreads;
	int m_next;
	std::vector<std::unique_ptr<EventLoopThread>> m_threads;
	std::vector<EventLoop* > m_loops;
};





} // namespace net







}// namespace dc
#endif

#include "asyncLogging.h"
#include "logFile.h"
#include "timeStamp.h"
#include "logging.h"
#include <cstdio>
#include "exception.h"

namespace dc
{
AsyncLogging::AsyncLogging(const string& basename, 
                           off_t rollSize,
                           int flushInterval)
    : m_flushInterval(flushInterval), 
      is_running(false),
      m_basename(basename),
      m_rollSize(rollSize),
      m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      m_latch(1),
      m_mutex(),
      m_cond(m_mutex),
      m_currentBuffer(new Buffer),
      m_nextBuffer(new Buffer),
      m_buffers()
{
    m_currentBuffer->bzero();
    m_nextBuffer->bzero();
    m_buffers.reserve(16);

}

void AsyncLogging::append(const char* logline, int len)
{
    dc::MutexLockGuard lock(m_mutex);
    
    if(!m_currentBuffer){
        //printf("first move\n");
        m_currentBuffer = std::move(m_nextBuffer);

    }

    if( m_currentBuffer->avail() > len )
    {
        m_currentBuffer->append(logline, len);
    }
    else
    {
        m_buffers.push_back(std::move(m_currentBuffer));
        if(m_nextBuffer)
        {
            //printf("move_next\n");
            m_currentBuffer = std::move(m_nextBuffer);
        }
        else
        {
            //printf("new current\n");
            m_currentBuffer.reset(new Buffer);
        }
        m_currentBuffer->append(logline, len);
        m_cond.notify();
    }
    
}

void AsyncLogging::threadFunc()
{
    assert(is_running == true);
    m_latch.countDown();
    LogFile output(m_basename, m_rollSize, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while(is_running)
    {
        assert(newBuffer1 && newBuffer1->length() == 0 );
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            MutexLockGuard lock(m_mutex);
            if(m_buffers.empty())
            {
                m_cond.waitForSeconds(m_flushInterval);
            }
            m_buffers.push_back(std::move(m_currentBuffer));
            m_currentBuffer = std::move(newBuffer1);
            buffersToWrite.swap(m_buffers);
            if(!m_nextBuffer)
            {
                m_nextBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if(buffersToWrite.size() > 25 )
        {
            LOG_INFO << "Dropped log message at here";
            char buf[256];
            snprintf(buf, sizeof(buf), "Dropped log message at %s, %zd larger buffers\n",
                    Timestamp::now().toFormattedString().c_str(), buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for( const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        if(buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if(!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if(!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();

    }
    output.flush();
}


}


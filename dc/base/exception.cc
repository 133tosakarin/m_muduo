#include "exception.h"
#include "currentThread.h"

namespace dc
{
Exception::Exception(std::string what) : m_message(std::move(what)), m_stack(CurrentThread::stackTrace(false))
{
	
}

}

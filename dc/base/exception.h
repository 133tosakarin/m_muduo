#ifndef DC_BASE_EXCEPTION_H
#define DC_BASE_EXCEPTION_H
#include "types.h"
#include <exception>

namespace dc
{

class Exception : public std::exception
{
public:
	Exception(std::string what);
	~Exception() noexcept override = default;

	const char* what() const noexcept override
	{
		return m_message.c_str();
	}

	const char* stackTrace() const noexcept
	{
		return m_stack.c_str();
	}
private:
	std::string m_message;
	std::string m_stack;

};

}

#endif

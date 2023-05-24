/*
 * =====================================================================================
 *
 *       Filename:  weakCallback.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/12/2023 02:43:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef DC_BASE_WEAKCALLBACK_H
#define DC_BASE_WEAKCALLBACK_H
#include <functional>
#include <memory>

namespace dc
{
template<typename CLASS, typename... ARGS>
class WeakCallback
{
public:
	WeakCallback(const std::weak_ptr<CLASS>& object,
				 const std::function<void (CLASS*, ARGS ...)>& func)
		: m_object(object), m_function(func)
	{

	}
	void operator()(ARGS&&... args) const
	{
		std::shared_ptr<CLASS> ptr(m_object.lock());
		if( ptr )
		{
			m_function(ptr.get(), std::forward<ARGS>(args)...);
		} 
	}

private:
	std::weak_ptr<CLASS> m_object;
	std::function<void(CLASS* , ARGS...)>m_function;


};
template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
											  void (CLASS::*function)(ARGS...))
{
	return WeakCallback<CLASS, ARGS...>(object, function);
}
template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
											  void (CLASS::*function)(ARGS...) const)
{
	return WeakCallback<CLASS, ARGS...>(object, function);
}
											  
											  

} // namespace dc
#endif

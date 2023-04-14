#ifndef DC_BASE_THREADLOCALSINGLETON
#define DC_BASE_THREADLOCALSINGLETON
#include "dc/base/noncopyable.h"

#include <assert.h>
#include <pthread.h>

namespace dc
{

template<typename T>
class ThreadLocalSingleton : noncopyable //every thread will havs this class 
{
public:

	ThreadLocalSingleton() = delete;
	~ThreadLocalSingleton() = delete;
	static T& instance()
	{
		if(!t_value)
		{
			t_value = new T();
			t_deletor.set(t_value);
		}
		return *t_value;
	}

	static T* pointer()
	{
		return t_value;
	}
private:
	static void destructor(void* obj)
	{
		assert(obj == t_value);
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		T_must_be_complete_type dummy;
		(void)dummy;
		delete t_value;
		t_value = nullptr;
	}
	class Deletor
	{
	public:

		Deletor()
		{
			pthread_key_create(&m_pkey,
					&ThreadLocalSingleton::destructor);
		}
		~Deletor()
		{
			pthread_key_delete(m_pkey);
		}

		void set(T* newObj)
		{
			assert(pthread_getspecific(m_pkey) == NULL);
			pthread_setspecific(m_pkey, newObj);
		}
		pthread_key_t m_pkey;
	};

	static thread_local T* t_value;
	static Deletor t_deletor;
};
template<typename T>
thread_local T* ThreadLocalSingleton<T>::t_value = nullptr;

template<typename T>
typename ThreadLocalSingleton<T>::Deletor ThreadLocalSingleton<T>::t_deletor;

}// namespace dc

#endif

#ifndef DC_BASE_THREADLOCAL_H
#define DC_BASE_THREADLOCAL_H

#include "mutex.h"
#include "noncopyable.h"
#include <pthread.h>

namespace dc
{

template<typename T>
class ThreadLocal : noncopyable
{
public:
    ThreadLocal()
    {
        MCHECK(pthread_key_create(&m_pkey, destructor));
    }
    ~ThreadLocal()
    {
        MCHECK(pthread_key_delete(m_pkey));
    }
    T& value()
    {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(m_pkey));
        if(!perThreadValue)
        {
            T* newobj = new T();
            MCHECK(pthread_setspecific(m_pkey, newobj));
            perThreadValue = newobj;
        }
        return *perThreadValue;
    }

    
private:
    static void destructor(void *x)
    {
        T* obj = static_cast<T*>(x);
       	typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void)dummy;
        delete obj;
    }
    pthread_key_t m_pkey;
};
}
#endif

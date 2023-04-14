#ifndef DC_BASE_SINGLETON_H
#define DC_BASE_SINGLETON_H

#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h> // atexit

namespace dc
{

namespace detail
{
// This doesn't detect inherited member functions!
//
template<typename T>
struct has_no_destroy
{
    template <typename C> static char test(decltype(&C::no_destroy));
    template <typename C> static int32_t test(...);
    const static bool value = sizeof(test<T>(0)) == 1;
};
template<typename T>
class Singleton : noncopyable
{
public:
    Singleton() = delete;
    ~Singleton() = delete;
    static T& instance()
    {
        pthread_once(&m_ponce, &Singleton::init);
        assert(m_value != nullptr );
        return *m_value;
    }
private:

    static void init()
    {
        m_value = new T();
        if( !detail::has_no_destroy<T>::value)
        {
            ::atexit(destroy);
        }
    } 
    static void destroy()
    {
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? - 1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete m_value;
        m_value = nullptr;
    }
    static pthread_once_t m_ponce;
    static T* m_value;
};

template<typename T>
pthread_once_t Singleton<T>::m_ponce = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::m_value = nullptr;
}
}

#endif

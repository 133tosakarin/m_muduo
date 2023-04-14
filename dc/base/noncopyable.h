
#ifndef DC_BASE_NONCOPYABLE_H
#define DC_BASE_NONCOPYABLE_H

namespace dc
{

class noncopyable
{
	public:
		noncopyable( const noncopyable&) = delete;
		void operator=(const noncopyable&) = delete;
		noncopyable( const noncopyable&& ) = delete;
	protected:
		noncopyable() = default;
		~noncopyable() = default;

};

}
#endif

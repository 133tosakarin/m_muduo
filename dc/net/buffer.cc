#include "dc/net/buffer.h"
#include "dc/net/socketsOps.h"

#include <errno.h>
#include <sys/uio.h>


using namespace dc;

const char net::Buffer::kCRLF[] = "\r\n";

const size_t net::Buffer::kCheapPrepend;
const size_t net::Buffer::kInitialSize;

ssize_t net::Buffer::readFd(int fd, int* savedErrno)
{
	// save an ioctl() /FIONREAD call to tell how much to read
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + m_writerIndex;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	// when there is enough space in this buffer, don't read into extrabuf.
	// when extrabuf is used, we read 128k - 1 bytes at most
	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
	const ssize_t n = sockets::readv(fd, vec, iovcnt);
	if( n < 0)
	{
		*savedErrno = errno;
	}
	else if( implicit_cast<size_t>(n) <= writable)
	{
		m_writerIndex +=n;
	}
	else
	{
		m_writerIndex = m_buffer.size();
		append(extrabuf, n - writable);
	}
	return n;
}


#include "fileUtil.h"
#include "logging.h"
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
namespace dc
{

AppendFile::AppendFile(StringArg filename) : m_fp(::fopen(filename.c_str(), "ar")), m_writtenBytes(0) //'e' for O_CLOEXEC
{
    assert(m_fp);
    ::setbuffer(m_fp, m_buffer, sizeof m_buffer);
}

AppendFile::~AppendFile()
{
    ::fclose(m_fp);
}

void AppendFile::append(const char* logline ,const size_t len)
{
    size_t written = 0;
    while( written != len )
    {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if( n != remain)
        {
            //len larger than 64k or some error
            int err = ferror(m_fp);
            if(err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
                break;
            }
        }
        written +=n;
    }
    m_writtenBytes +=written;
}

void AppendFile::flush()
{
    ::fflush(m_fp);
}

size_t AppendFile::write(const char* logline , size_t len)
{
    return ::fwrite_unlocked(logline, 1, len, m_fp);
}

ReadSmallFile::ReadSmallFile(StringArg filename) : m_fd(::open(filename.c_str(), O_RDONLY | O_CLOEXEC )), m_err(0)
{
    m_buf[0] = '\0';
    if( m_fd < 0 )
    {
        m_err = errno;
    }
}

ReadSmallFile::~ReadSmallFile()
{
    if( m_fd >= 0)
    {
        ::close(m_fd);
    }
}

template<typename String>
int ReadSmallFile::readToString(int maxSize,
                                String* content,
                                int64_t* fileSize,
                                int64_t* modifyTime,
                                int64_t* createTime)
{
    static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
    assert(content != nullptr);
    int err = m_err;
    if( m_fd >= 0)
    {
        content->clear();
        if (fileSize)
        {
            struct stat statbuf;
            if( ::fstat(m_fd, &statbuf) == 0 )
            {
                if( S_ISREG(statbuf.st_mode))
                {
                    *fileSize = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
                }
                else if(S_ISDIR(statbuf.st_mode))
                {
                    err = EISDIR;
                }
                if(modifyTime)
                {
                    *modifyTime = statbuf.st_ctime;
                }
                if(createTime)
                {
                    *createTime = statbuf.st_ctime;
                }
            }
            else
            {
                err = errno;
            }
        }

        while(content->size() < implicit_cast<size_t>(maxSize))
        {
            size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(m_buf));
            ssize_t n = ::read(m_fd, m_buf, toRead);
            if( n > 0)
            {
                content->append(m_buf, n);
            }
            else
            {
                if( n < 0)
                {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

int ReadSmallFile::readToBuffer(int* size)
{
    int err = m_err;
    if( m_fd >= 0 )
    {
        ssize_t n = ::pread(m_fd, m_buf, sizeof(m_buf) - 1, 0);
        if( n >= 0)
        {
            if(size)
            {
                *size = static_cast<int>(n);

            }
            m_buf[n] = 0;
        }
    }
    else
    {
        err = errno;
    }
    return err;
}

template int readFile(StringArg filename,
                        int maxSize,
                        std::string* content,
                        int64_t*, int64_t*, int64_t*);

template int ReadSmallFile::readToString(int maxSize,
                                         std::string* content,
                                        int64_t*, int64_t*, int64_t*);
}
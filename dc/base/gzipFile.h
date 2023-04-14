#ifndef DC_BASE_GZIPFILE_H
#define DC_BASE_GZIPFILE_H

#include "stringPieces.h"
#include "noncopyable.h"
#include <zlib.h>

namespace dc
{

class GzipFile : noncopyable
{
public:
    GzipFile(GzipFile&& rhs) noexcept
        : m_file(rhs.m_file)
        {
            rhs.file = NULL;
        }
    ~GzipFile()
    {
        if(m_file)
        {
            ::gzclose(m_file);
        }
    }

    GzipFile& operator=(GzipFile&& rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
    bool valid() const {return m_file != NULL; }
    void swap(GzipFile& rhs) { std::swap(m_file, rhs.m_file); }
#if ZLIB_VERNUM >= 0x1240
    bool setBuffer(int size) { return ::gzbuffer(m_file, size) == 0; }
#endif
    // return the number of uncompressed bytes actually read, 0 for eof, -1 for error

    int read(void* buf, len) { return ::gzwrite(m_file, buf, len);}
    //return the nunber of uncompressed bytes actuall written
    int write(StringPiece buf) { return ::gzwrite(m_file, buf.data(), buf.size()); }

    // number of uncompressd bytes
    off_t tell() const { return ::gztell(m_file); }

#if ZLIB_BERNUM >= 0x1240
    //number of compressed bytes
    off_t offset() const { return ::gzoffset(m_file); }
#endif

static GzipFile openForRead(StringArg filename)
{
    return GzipFile(::gzopen(filename.c_str(), "rbe"));
}

static GzipFile openForAppend(StringArg filename)
{
    return GzipFile(::gzopen(filename.c_str(), "abe"));
}

static GzipFile openForWriteExclusive(StringArg filename)
{
    return GzipFile(::gzopen(filename.c_str(), "wbxe"));
}

static GzipFile openForWriteTruncate(StringArg filename)
{
    return GzipFile(::gzopen(filename.c_str(), "wbe"));
}


private:
    explicit GzipFile(gzfile file) : m_file(file) {}
    gzFile m_file;
};
}

#endif
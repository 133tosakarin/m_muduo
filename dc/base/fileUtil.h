#ifndef DC_BASE_FILEUTIL_H
#define DC_BASE_FILEUTIL_H

#include "noncopyable.h"
#include "stringPieces.h"


namespace dc
{

class ReadSmallFile : noncopyable
{
public:
    ReadSmallFile(StringArg filename);
    ~ReadSmallFile();

    // return errno
    template<typename String>
    int readToString(int maxSize,
                    String* content,
                    int64_t* fileSize,
                    int64_t* modifyTime,
                    int64_t* createTime);

    // read at maxium kBufferSize into m_buf
    int readToBuffer(int* size);
    const char* buffer() const { return m_buf; }


    static const int kBufferSize = 64 * 1024;
private:
    int m_fd;
    int m_err;
    char m_buf[kBufferSize];
};

template<typename String>
int readFile(StringArg filename,
            int maxSize,
            String* content,
            int64_t* fileSize = nullptr,
            int64_t* modifyTime = nullptr,
            int64_t* createTime = nullptr)
{
    ReadSmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

//not thread safe

class AppendFile: noncopyable
{

public:
    explicit AppendFile(StringArg filename);
    ~AppendFile();
    void append(const char* logline, size_t len);
    void flush();

    off_t writtenBytes() const { return m_writtenBytes; }

private:
    size_t write(const char* logline, size_t len);

    FILE* m_fp;
    char m_buffer[64 * 1024];
    off_t m_writtenBytes;
};

}
#endif
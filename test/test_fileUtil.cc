#include "../dc/base/fileUtil.h"
#include <cstdio>
#include <iostream>
#include <inttypes.h>

using namespace dc;
int main()
{
    string result;
    int64_t size = 0;
    int err = readFile("/proc/self", 1024, &result, &size);
    printf("%d %zd %" PRIu64 " %s\n", err, result.size(), size, strerror(err) );
    std::cout << result << std::endl;
    err = readFile("/proc/self", 1024, &result, nullptr);
    std::cout << result << std::endl;
    printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
    std::cout << result << std::endl;
    err = readFile("/proc/self/cmdline", 1024, &result, &size);
    std::cout << result << std::endl;
    printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
    std::cout << result << std::endl;
    err = readFile("dev/null", 1024, &result, &size);
    std::cout << result << std::endl;
    printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
    err = readFile("dev/zero", 1024, &result, &size);
    printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
    err = readFile("/notexist", 1024, &result, &size);
    printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
    int64_t modifytime, createTime;
    err = readFile("/dev/zero", 102400, &result,&size, &modifytime, &createTime);
    printf("%d %zd %" PRIu64 " %lu %lu\n", err, result.size(), size, modifytime, createTime);
    return 0;
}

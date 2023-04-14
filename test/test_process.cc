#include "../dc/base/processInfo.h"
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

int main()
{
  printf("pid = %d\n", dc::ProcessInfo::pid());
  printf("uid = %d\n", dc::ProcessInfo::uid());
  printf("euid = %d\n", dc::ProcessInfo::euid());
  printf("start time = %s\n", dc::ProcessInfo::startTime().toFormattedString().c_str());
  printf("hostname = %s\n", dc::ProcessInfo::hostname().c_str());
  printf("opened files = %d\n", dc::ProcessInfo::openedFiles());
  printf("threads = %zd\n", dc::ProcessInfo::threads().size());
  printf("num threads = %d\n", dc::ProcessInfo::numThreads());
  printf("status = %s\n", dc::ProcessInfo::procStatus().c_str());
}

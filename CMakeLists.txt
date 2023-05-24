CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(dc)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic   -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -lyaml-cpp -Wno-builtin-macro-redefined -lboost_unit_test_framework -lboost_program_options -lboost_system ")


include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)

find_package(Boost REQUIRED)
find_package(Protobuf)
find_package(CURL)
find_package(ZLIB)
find_path(CARES_INCLUDE_DIR ares.h)
find_library(CARES_LIBRARY NAMES cares)
find_path(MHD_INCLUDE_DIR microhttpd.h)
find_library(MHD_LIBRARY NAMES microhttpd)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
find_library(BOOSTSYSTEM_LIBRARY NAMES boost_system)

if(CARES_INCLUDE_DIR AND CARES_LIBRARY)
  message(STATUS "found cares")
endif()
if(CURL_FOUND)
  message(STATUS "found curl")
endif()
if(PROTOBUF_FOUND)
  message(STATUS "found protobuf")
endif()
if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  message(STATUS "found tcmalloc")
endif()
if(ZLIB_FOUND)
  message(STATUS "found zlib")
endif()
if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
  message(STATUS "found hiredis")
endif()
if(GD_INCLUDE_DIR AND GD_LIBRARY)
  message(STATUS "found gd")
endif()
if(THRIFT_COMPILER AND THRIFT_INCLUDE_DIR AND THRIFT_LIBRARY)
  message(STATUS "found thrift")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/dc/base)
#include_directories(${PROJECT_SOURCE_DIR}/dc/net)


message("***",${YAMLCPP})
set(LIB_SRC
	dc/base/timeStamp.cc
	dc/base/timeZone.cc
	dc/base/condition.cc
	dc/base/exception.cc
	dc/base/currentThread.cc
	dc/base/countDownLatch.cc
	dc/base/logging.cc
	dc/base/logStream.cc
	dc/base/date.cc
	dc/base/processInfo.cc
	dc/base/fileUtil.cc
	dc/base/thread.cc
	dc/base/logFile.cc
	dc/base/threadpool.cc
	dc/base/asyncLogging.cc
	dc/net/eventLoop.cc
	dc/net/channel.cc
	dc/net/buffer.cc
	dc/net/socketsOps.cc
	dc/net/timer.cc
	dc/net/poller.cc
	dc/net/socket.cc
	dc/net/inetAddress.cc
	dc/net/timerQueue.cc
	dc/net/poller/pollPoller.cc
	dc/net/poller/epollPoller.cc
	dc/net/poller/defaultPoller.cc
	dc/net/tcpServer.cc
	dc/net/acceptor.cc
	dc/net/connector.cc
	dc/net/tcpClient.cc
	dc/net/eventLoopThread.cc
	dc/net/eventLoopThreadPool.cc
	dc/net/tcpConnection.cc
	dc/net/http/httpServer.cc
	dc/net/http/httpContext.cc
	dc/net/http/httpResponse.cc
	)

add_library(dc SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(dc) #__FILE__
#add_library(dc_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (dc_static PROPERTIES OUTPUT_NAME "dc")

set(LIB_LIB
	dc
	pthread
	${YAMLCPP}
	)







add_executable(test_log test/test_log.cc)
add_dependencies(test_log dc)
force_redefine_file_macro_for_sources(test_log) #__FILE__
target_link_libraries(test_log ${LIB_LIB})


add_executable(test_thread test/test_thread.cc)
add_dependencies(test_thread dc)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread ${LIB_LIB})

add_executable(test_threadpool test/test_threadpool.cc)
add_dependencies(test_threadpool dc)
force_redefine_file_macro_for_sources(test_threadpool) #__FILE__
target_link_libraries(test_threadpool ${LIB_LIB})


add_executable(test_fileUtil test/test_fileUtil.cc)
add_dependencies(test_fileUtil dc)
force_redefine_file_macro_for_sources(test_fileUtil) #__FILE__
target_link_libraries(test_fileUtil ${LIB_LIB})


add_executable(test_asyncLogging test/test_asyncLogging.cc)
add_dependencies(test_asyncLogging dc)
force_redefine_file_macro_for_sources(test_asyncLogging) #__FILE__
target_link_libraries(test_asyncLogging ${LIB_LIB})

add_executable(test_mutex test/test_mutex.cc)
add_dependencies(test_mutex dc)
force_redefine_file_macro_for_sources(test_mutex) #__FILE__
target_link_libraries(test_mutex ${LIB_LIB})

add_executable(test_process test/test_process.cc)
add_dependencies(test_process dc)
force_redefine_file_macro_for_sources(test_process) #__FILE__
target_link_libraries(test_process ${LIB_LIB})

add_executable(test_logstream test/test_logstream.cc)
add_dependencies(test_logstream dc)
force_redefine_file_macro_for_sources(test_logstream) #__FILE__
target_link_libraries(test_logstream ${LIB_LIB})

add_executable(test_timestamp test/test_timestamp.cc)
add_dependencies(test_timestamp dc)
target_link_libraries(test_timestamp ${LIB_LIB})

add_executable(test_atomic test/test_atomic.cc)
target_link_libraries(test_atomic ${LIB_LIB})

add_executable(test_exception test/test_exception.cc)
target_link_libraries(test_exception ${LIB_LIB})

add_executable(test_blockingQueue test/test_blockingQueue.cc)
target_link_libraries(test_blockingQueue ${LIB_LIB})

add_executable(test_blockingQueue_bench test/test_blockingQueue_bench.cc)
target_link_libraries(test_blockingQueue_bench ${LIB_LIB})

add_executable(test_boundQueue test/test_boundQueue.cc)
target_link_libraries(test_boundQueue ${LIB_LIB})

add_executable(test_singleton test/test_singleton.cc)
target_link_libraries(test_singleton ${LIB_LIB})

add_executable(test_threadLocal test/test_threadLocal.cc)
target_link_libraries(test_threadLocal ${LIB_LIB})


add_executable(test_singletonThreadLocal test/test_singletonThreadLocal.cc)
target_link_libraries(test_singletonThreadLocal ${LIB_LIB})

add_executable(test_logFile test/test_logFile.cc)
target_link_libraries(test_logFile ${LIB_LIB})

add_executable(test_logging test/test_logging.cc)
target_link_libraries(test_logging ${LIB_LIB})

add_executable(test_buffer test/test_buffer.cc)
target_link_libraries(test_buffer ${LIB_LIB})

add_executable(test_channel test/test_channel.cc)
target_link_libraries(test_channel ${LIB_LIB})

add_executable(test_inetAddress test/test_inetAddress.cc)
target_link_libraries(test_inetAddress ${LIB_LIB})

add_executable(test_eventLoop test/test_eventLoop.cc)
target_link_libraries(test_eventLoop ${LIB_LIB})

add_executable(test_eventLoopThread test/test_eventLoopThread.cc)
target_link_libraries(test_eventLoopThread ${LIB_LIB})

add_executable(test_eventLoopThreadPool test/test_eventLoopThreadPool.cc)
target_link_libraries(test_eventLoopThreadPool ${LIB_LIB})

add_executable(test_timerQueue test/test_timerQueue.cc)
target_link_libraries(test_timerQueue ${LIB_LIB})

add_executable(test_echoServer test/test_echoServer.cc)
target_link_libraries(test_echoServer ${LIB_LIB})

add_executable(test_echoClient test/test_echoClient.cc)
target_link_libraries(test_echoClient ${LIB_LIB})

add_executable(test_httpRequest test/test_httpRequest.cc)
target_link_libraries(test_httpRequest ${LIB_LIB})

add_executable(test_testHttp test/test_testHttp.cc)
target_link_libraries(test_testHttp ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)





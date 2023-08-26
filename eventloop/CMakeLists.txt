cmake_minimum_required(VERSION 3.0)
project(eventloop)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_EVENTLOOP_EXAMPLES "build examples" OFF)

set(LIB_SRC
    channel.cxx
    event_loop.cxx
    poller.cxx
    timer.cxx
    timespan.cxx
    timestamp.cxx
    timespec.cxx
    this_thread.cxx
    proto_codec.cxx)

add_library(eventloop STATIC ${LIB_SRC})
target_compile_options(eventloop PRIVATE -Wno-psabi)
target_include_directories(eventloop PUBLIC ${MUDUO_TOP})
# target_link_libraries(eventloop PUBLIC rt)

if(BUILD_EVENTLOOP_EXAMPLES)
  add_executable(test_timer example/test_timer.cxx)
  target_link_libraries(test_timer eventloop)

  add_executable(test_timer2 example/test_timer2.cxx)
  target_link_libraries(test_timer2 eventloop)

  add_executable(test_ts example/test_timestamp.cxx)
  target_link_libraries(test_ts eventloop)
endif()

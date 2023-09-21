cmake_minimum_required(VERSION 3.0)
project(eventloop)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

option(ENABLE_COMPONENT_PROTOBUF "enable component protobuf" OFF)
option(EVENTLOOP_USE_MUDUO_LOGGER "use muduo logger in eventloop" OFF)

set(EVENTLOOP_SRC
    channel.cxx
    event_loop.cxx
    poller.cxx
    timer.cxx
    timer_queue.cxx
    timespan.cxx
    timestamp.cxx
    timespec.cxx
    this_thread.cxx
    event_loop_thread.cxx
    event_loop_threadpool.cxx)

if(ENABLE_COMPONENT_PROTOBUF)
  list(APPEND EVENTLOOP_SRC proto_codec.cxx)

  find_package(Protobuf REQUIRED)
  if(PROTOBUF_FOUND)
    message(STATUS "protobuf found, ${Protobuf_PROTOC_EXECUTABLE}")
  else()
    message(FATAL_ERROR "protobuf library is not found")
  endif()
endif()

add_library(eventloop STATIC ${EVENTLOOP_SRC})
target_compile_options(eventloop PRIVATE -Wno-psabi)
target_include_directories(eventloop PUBLIC ${MUDUO_TOP})
if(EVENTLOOP_USE_MUDUO_LOGGER)
  target_compile_definitions(eventloop PRIVATE EVENTLOOP_USE_MUDUO_LOGGER)
endif()
# target_link_libraries(eventloop PUBLIC rt)
if(PROTOBUF_FOUND)
  target_link_libraries(eventloop PUBLIC ${PROTOBUF_LIBRARIES})
endif()

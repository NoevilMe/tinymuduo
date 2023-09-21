cmake_minimum_required(VERSION 3.0)
project(muduo_logger)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

option(BUILD_MUDUO_LOG_EXAMPLES "build muduo log examples" OFF)

set(MUDUO_LOG_SRC log_stream.cxx log_file.cxx logger.cxx)
add_library(muduo_logger STATIC ${MUDUO_LOG_SRC})

if(BUILD_MUDUO_LOG_EXAMPLES)
  add_executable(test_logger test_logger.cpp)
  target_link_libraries(test_logger PRIVATE muduo_logger pthread)
endif()

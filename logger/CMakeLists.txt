cmake_minimum_required(VERSION 3.0)
project(muduo_logger)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(LIB_LOG_SRC log_stream.cxx log_file.cxx logger.cxx)
add_library(muduo_logger STATIC ${LIB_LOG_SRC})

add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger PRIVATE muduo_logger pthread)

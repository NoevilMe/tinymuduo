cmake_minimum_required(VERSION 3.0)
project(eventloop)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# target_compile_options(eventloop PRIVATE -Wno-psabi)
# target_link_libraries(eventloop PUBLIC rt)

set(LIB_LOG_SRC log_stream.cxx log_file.cxx logger.cxx)
add_library(muduo_logger STATIC ${LIB_LOG_SRC})

add_executable(test test.cpp)
target_link_libraries(test PRIVATE muduo_logger pthread)

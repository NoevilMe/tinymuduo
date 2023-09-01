cmake_minimum_required(VERSION 3.0)
project(tinymuduo)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_EVENTLOOP_EXAMPLES "build examples" OFF)

# set(CMAKE_CXX_FLAGS "-g")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(logger)
add_subdirectory(eventloop)

set(NET_SRC net/inet_socket.cxx net/inet_address.cxx net/acceptor.cxx
            net/tcp_connection.cxx net/tcp_server.cxx net/buffer.cxx)
add_library(muduo_net ${NET_SRC})
target_link_libraries(muduo_net PUBLIC eventloop muduo_logger)

add_executable(test_net example/test_acceptor.cxx)
target_link_libraries(test_net PRIVATE muduo_net pthread)

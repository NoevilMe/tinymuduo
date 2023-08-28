cmake_minimum_required(VERSION 3.0)
project(tinymuduo)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_EVENTLOOP_EXAMPLES "build examples" OFF)

set(CMAKE_CXX_FLAGS "-g")

set(MUDUO_TOP ${CMAKE_SOURCE_DIR})

add_subdirectory(logger)
add_subdirectory(eventloop)

set(NET_SRC net/inet_socket.cxx net/inet_address.cxx net/mem.cxx
            net/acceptor.cxx net/tcp_connection.cxx net/tcp_server.cxx net/buffer.cxx)
add_library(muduo_net ${NET_SRC})
target_link_libraries(muduo_net PRIVATE eventloop)
target_include_directories(muduo_net PUBLIC ${MUDUO_TOP})

add_executable(test_net example/test_acceptor.cxx)
target_link_libraries(test_net PRIVATE muduo_net eventloop muduo_logger)

cmake_minimum_required(VERSION 3.0)
project(tinymuduo)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_TINYMODUO_EXAMPLES "build examples" OFF)

set(CMAKE_CXX_FLAGS "-g -O0")

set(EVENTLOOP_USE_MUDUO_LOGGER
    ON
    CACHE BOOL "enable muduo logger in eventloop component")

# set(CMAKE_CXX_FLAGS "-g")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(logger)
add_subdirectory(eventloop)

set(MUDUO_NET_SRC
    net/inet_socket.cxx
    net/inet_address.cxx
    net/acceptor.cxx
    net/tcp_connection.cxx
    net/tcp_server.cxx
    net/buffer.cxx
    net/udp_server.cxx
    net/udp_virtual_connection.cxx)
add_library(muduo_net ${MUDUO_NET_SRC})
target_link_libraries(muduo_net PUBLIC eventloop muduo_logger)

if(BUILD_TINYMODUO_EXAMPLES)
  add_executable(test_timer example/test_timer.cxx)
  target_link_libraries(test_timer PRIVATE eventloop)
  if(EVENTLOOP_USE_MUDUO_LOGGER)
    target_link_libraries(test_timer PRIVATE muduo_logger)
  endif()

  add_executable(test_tcp_server example/test_tcp_server.cxx)
  target_link_libraries(test_tcp_server PRIVATE muduo_net pthread)

  add_executable(test_udp_server example/test_udp_server.cxx)
  target_link_libraries(test_udp_server PRIVATE muduo_net pthread)

  add_executable(test_udp_conn example/test_udp_conn.cxx)
  target_link_libraries(test_udp_conn PRIVATE muduo_net pthread)

endif()

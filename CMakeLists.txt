set(LIB_SRC
    channel.cxx
    event_loop.cxx
    poller.cxx
    timer.cxx
    timespan.cxx
    timestamp.cxx
    )

add_library(eventloop STATIC ${LIB_SRC})

add_executable(test example/test_timer.cxx)
target_link_libraries(test eventloop)

add_executable(test2 example/test_timer2.cxx)
target_link_libraries(test2 eventloop)
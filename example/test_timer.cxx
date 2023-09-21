#include "eventloop/event_loop.h"
#include "eventloop/timer.h"

#include <iostream>

int main() {

    muduo::event_loop::EventLoop loop;

    // run at
    auto now = muduo::event_loop::Timestamp::Now();
    std::cout << now.ToFormattedMicroSecondsString() << std::endl;

    loop.RunAt(now + 5, []() {
        std::cout << "timer 1, run at 5 later, "
                  << muduo::event_loop::Timestamp::Now()
                         .ToFormattedMicroSecondsString()
                  << std::endl;
    });
    // run after
    loop.RunAfter(6, []() {
        std::cout << "timer 2, run after 6s, "
                  << muduo::event_loop::Timestamp::Now()
                         .ToFormattedMicroSecondsString()
                  << std::endl;
    });

    // run every
    loop.RunEvery(7, []() {
        std::cout << "timer 3, run every 7s, "
                  << muduo::event_loop::Timestamp::Now()
                         .ToFormattedMicroSecondsString()
                  << std::endl;
    });

    // run every after
    loop.RunEveryAfter(7, 10, []() {
        std::cout << "timer 4, run every 7s after 10s, "
                  << muduo::event_loop::Timestamp::Now()
                         .ToFormattedMicroSecondsString()
                  << std::endl;
    });

    // run every at
    loop.RunEveryAt(20, now + 10, []() {
        std::cout << "timer 5, run every 20s at 10s later, "
                  << muduo::event_loop::Timestamp::Now()
                         .ToFormattedMicroSecondsString()
                  << std::endl;
    });

    loop.Loop();

    return 0;
}
#include "../event_loop.h"
#include "../timer.h"

#include <iostream>

int main() {
    muduo::event_loop::EventLoop loop;
    std::cout << "start "
              << muduo::event_loop::Timestamp::Now().milliseconds_since_epoch()
              << std::endl;

    loop.RunEveryAfter(5, 20, []() {
        std::cout
            << "hello 1 -------- "
            << muduo::event_loop::Timestamp::Now().milliseconds_since_epoch()
            << std::endl;
    });

    loop.RunAfter(2.5 ,[]() {
        std::cout
            << "hello 2 ----- "
            << muduo::event_loop::Timestamp::Now().milliseconds_since_epoch()
            << std::endl;
    });

    loop.Loop();

    return 0;
}
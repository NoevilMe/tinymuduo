#include "../event_loop.h"
#include "../timer.h"

#include <iostream>

int main() {

    muduo::event_loop::EventLoop loop;
    std::cout << "start "
              << muduo::event_loop::Timestamp::Now().MillisecondsSinceEpoch()
              << std::endl;
    muduo::event_loop::Timer t(
        &loop,
        []() {
            std::cout << "hello "
                      << muduo::event_loop::Timestamp::Now()
                             .MillisecondsSinceEpoch()
                      << std::endl;
        },
        20, 5);

    muduo::event_loop::Timer t2(
        &loop,
        []() {
            std::cout << "2 "
                      << muduo::event_loop::Timestamp::Now()
                             .MillisecondsSinceEpoch()
                      << std::endl;
        },
        5.5, 0);

    muduo::event_loop::Timer t3(
        &loop,
        []() {
            std::cout << "3 ----- "
                      << muduo::event_loop::Timestamp::Now()
                             .MillisecondsSinceEpoch()
                      << std::endl;
        },
        muduo::event_loop::Timestamp::Now() + 2, 3);

    // t.set_expired_callback([]() { std::cout << "t is expired" << std::endl;
    // }); t2.set_expired_callback([]() { std::cout << "t2 is expired" <<
    // std::endl; }); t3.set_expired_callback([]() { std::cout << "t3 is
    // expired" << std::endl; });
    loop.Loop();

    return 0;
}
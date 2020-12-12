#include "../timestamp.h"

#include <iostream>

using namespace muduo::event_loop;

int main() {
    Timestamp ts = Timestamp::Now();
    std::cout << "seconds: " << ts.seconds_since_epoch() << std::endl;
    std::cout << "milliseconds: " << ts.milliseconds_since_epoch() << std::endl;
    std::cout << "microseconds: " << ts.microseconds_since_epoch() << std::endl;
    std::cout << "nanoseconds: " << ts.nanoseconds_since_epoch() << std::endl;
    std::cout << "string: " << ts.ToString() << std::endl;
    std::cout << "millistring: " << ts.ToFormattedMilliSecondsString() << std::endl;
    std::cout << "microstring: " << ts.ToFormattedMicroSecondsString() << std::endl;
    std::cout << "nanostring: " << ts.ToFormattedString() << std::endl;

    return 0;
}
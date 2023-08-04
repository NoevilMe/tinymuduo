#include "../timestamp.h"

#include <iostream>

using namespace muduo::event_loop;

int main() {
    Timestamp ts = Timestamp::Now();
    std::cout << "seconds: " << ts.SecondsSinceEpoch() << std::endl;
    std::cout << "milliseconds: " << ts.MillisecondsSinceEpoch() << std::endl;
    std::cout << "microseconds: " << ts.MicrosecondsSinceEpoch() << std::endl;
    std::cout << "nanoseconds: " << ts.NanosecondsSinceEpoch() << std::endl;
    std::cout << "string: " << ts.ToString() << std::endl;
    std::cout << "millistring: " << ts.ToFormattedMilliSecondsString() << std::endl;
    std::cout << "microstring: " << ts.ToFormattedMicroSecondsString() << std::endl;
    std::cout << "nanostring: " << ts.ToFormattedString() << std::endl;

    return 0;
}
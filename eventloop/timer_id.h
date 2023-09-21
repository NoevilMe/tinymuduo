#ifndef __MUDUO_TIMER_ID_H_
#define __MUDUO_TIMER_ID_H_

#include <cstdint>

namespace muduo {
namespace event_loop {

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId {
public:
    TimerId() : timer_(nullptr), sequence_(0) {}

    TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    Timer *timer_;
    int64_t sequence_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMER_ID_H_ */

#ifndef __MUDUO_TIMER_H_
#define __MUDUO_TIMER_H_

#include "callback.h"
#include "noncopyable.h"
#include "timestamp.h"

#include <atomic>
#include <memory>

namespace muduo {
namespace event_loop {

class EventLoop;
class Channel;
class Timestamp;

class Timer : Noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : cb_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(++s_num) {}
    ~Timer() {}

    void Run() const { cb_(); }

    void Restart(Timestamp now);

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    static int64_t Num() { return s_num; }

private:
    const TimerCallback cb_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_int64_t s_num;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMER_H_ */

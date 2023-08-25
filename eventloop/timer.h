#ifndef __MUDUO_TIMER_H_
#define __MUDUO_TIMER_H_

#include "callback.h"

#include <memory>

namespace muduo {
namespace event_loop {

class EventLoop;
class Channel;
class Timestamp;

class Timer : public std::enable_shared_from_this<Timer> {
public:
    Timer(EventLoop *eventloop, TimerCallback cb, Timestamp when,
          double interval_seconds);
    explicit Timer(EventLoop *eventloop, TimerCallback cb, double delay_seconds,
                   double interval_seconds);
    ~Timer();

    void Stop();

    bool Expired();

    int fd() const { return timer_fd_; }

    void Postpone(double seconds);

    void PostponeAfter(double seconds);

private:
    void InitTimer(double delay_seconds, double interval_seconds);

    void ReadTimer();

    EventLoop *eventloop_;
    TimerCallback cb_;
    int timer_fd_;
    std::unique_ptr<Channel> timer_channel_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMER_H_ */

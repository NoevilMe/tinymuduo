#ifndef __TIMER_H_
#define __TIMER_H_

#include "event_base.h"
#include <memory>

namespace muduo {
namespace event_loop {

class EventLoop;
class Channel;

class Timer {
public:
    Timer(EventLoop *eventloop, TimerCallback cb, int interval);
    ~Timer();

    void Stop();

private:
    void ReadTimer();

    EventLoop *eventloop_;
    TimerCallback cb_;
    int timer_fd_;
    std::unique_ptr<Channel> timer_channel_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __TIMER_H_ */

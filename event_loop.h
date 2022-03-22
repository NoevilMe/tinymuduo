#ifndef __MUDUO_EVENT_LOOP_H_
#define __MUDUO_EVENT_LOOP_H_

#include "callback.h"
#include "noncopyable.h"
#include "timestamp.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <vector>

namespace muduo {
namespace event_loop {

class Channel;
class Poller;
class Timer;

class EventLoop {
public:
    using ChannelList = std::vector<Channel *>;

    EventLoop();
    ~EventLoop();

    ///
    /// Loops forever.
    ///
    /// Must be called in the same thread as creation of the object.
    ///
    void Loop();

    /// Quits loop.
    ///
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void Quit();

    void RunInLoop(Functor cb);
    void QueueInLoop(Functor cb);

    // timers

    std::shared_ptr<Timer> RunAt(Timestamp time, TimerCallback cb);

    std::shared_ptr<Timer> RunAfter(double delay, TimerCallback cb);

    std::shared_ptr<Timer> RunEvery(double interval, TimerCallback cb);

    std::shared_ptr<Timer> RunEveryAfter(double interval, double delay,
                                         TimerCallback cb);

    std::shared_ptr<Timer> RunEveryAt(double interval, Timestamp time,
                                      TimerCallback cb);

    void RemoveTimer(int timer_fd);
    std::size_t TimerCount();

    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);

private:
    std::mutex pending_functors_mutex_;
    std::vector<Functor> pending_functors_;

    std::unique_ptr<Poller> poller_;
    bool quit_;
    ChannelList active_channels_;
    Channel *current_channel_;
    Timestamp poll_timestamp_;

    std::map<int, std::shared_ptr<Timer>> timers_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_EVENT_LOOP_H_ */

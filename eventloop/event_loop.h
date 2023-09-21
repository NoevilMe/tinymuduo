#ifndef __MUDUO_EVENT_LOOP_H_
#define __MUDUO_EVENT_LOOP_H_

#include "callback.h"
#include "noncopyable.h"
#include "this_thread.h"
#include "timer_id.h"
#include "timestamp.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <sys/epoll.h>

namespace muduo {
namespace event_loop {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : public Noncopyable {
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

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void RunInLoop(Functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void QueueInLoop(Functor cb);

    // timers

    ///
    /// Runs callback at 'time'.
    /// Safe to call from other threads.
    ///
    TimerId RunAt(Timestamp time, TimerCallback cb);
    ///
    /// Runs callback after @c delay seconds.
    /// Safe to call from other threads.
    ///
    TimerId RunAfter(double delay, TimerCallback cb);
    ///
    /// Runs callback every @c interval seconds.
    /// Safe to call from other threads.
    ///
    TimerId RunEvery(double interval, TimerCallback cb);
    TimerId RunEveryAfter(double interval, double delay, TimerCallback cb);
    TimerId RunEveryAt(double interval, Timestamp time, TimerCallback cb);
    ///
    /// Cancels the timer.
    /// Safe to call from other threads.
    ///
    void Cancel(TimerId timer_id);

    // void RemoveTimer(int timer_fd);
    // std::size_t TimerCount();

    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);
    void Wakeup();

    bool event_handling() const { return event_handling_; }

    bool IsInLoopThread() const { return thread_id_ == this_thread::tid(); }
    void AssertInLoopThread() {
        if (!IsInLoopThread()) {
            AbortNotInLoopThread();
        }
    }

    static EventLoop *GetEventLoopOfThisThread();

private:
    void AbortNotInLoopThread();
    void CallPendingFunctors();
    void WakeUpEventRead(Timestamp); // waked up

private:
    const pid_t thread_id_;
    bool looping_;
    bool event_handling_;
    std::atomic_bool quit_;

    // pending functors related
    bool calling_pending_functors_;
    std::mutex pending_functors_mutex_;
    std::vector<Functor> pending_functors_;

    std::unique_ptr<Poller> poller_;

    ChannelList active_channels_;
    Channel *current_channel_;
    Timestamp poll_timestamp_;

    int wakeup_fd_;
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    std::unique_ptr<Channel> wakeup_channel_;

    std::unique_ptr<TimerQueue> timer_queue_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_EVENT_LOOP_H_ */

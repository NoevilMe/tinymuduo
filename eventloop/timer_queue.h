#ifndef __MUDUO_TIMER_QUEUE_H_
#define __MUDUO_TIMER_QUEUE_H_

#include "callback.h"
#include "channel.h"
#include "noncopyable.h"
#include "timer_id.h"
#include "timestamp.h"

#include <set>
#include <vector>

namespace muduo {
namespace event_loop {

class EventLoop;
class Timer;

class TimerQueue : Noncopyable {
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);

    void Cancel(TimerId timerId);

private:
    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    // This requires heterogeneous comparison lookup (N3465) from C++14
    // so that we can find an T* in a set<unique_ptr<T>>.
    using Entry = std::pair<Timestamp, Timer *>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer *, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void AddTimerInLoop(Timer *timer);
    bool Insert(Timer *timer);
    void CancelInLoop(TimerId timer_id);

    // called when timerfd alarms
    void HandleRead();
    // move out all expired timers
    std::vector<Entry> GetExpired(Timestamp now);
    void Reset(const std::vector<Entry> &expired, Timestamp now);

private:
    EventLoop *loop_;
    const int timer_fd_;
    std::unique_ptr<Channel> channel_;
    TimerList timers_;

    // for cancel()
    ActiveTimerSet active_timers_;
    bool calling_expired_timers_; /* atomic */
    ActiveTimerSet canceling_timers_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMER_QUEUE_H_ */

#include "timer_queue.h"
#include "event_loop.h"
#include "import_log.h"
#include "timer.h"

#include <assert.h>
#include <cstring>
#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo {
namespace event_loop {

namespace details {

int CreateTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    return timerfd;
}

void ResetTimerfd(int timerfd, Timestamp expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    ::bzero(&new_value, sizeof new_value);
    ::bzero(&old_value, sizeof old_value);
    new_value.it_value = Timestamp::HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    (void)ret;
}

void ReadTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "
              << now.ToString();
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerQueue::HandleRead() reads " << n
                  << " bytes instead of 8";
    }
}

} // namespace details

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timer_fd_(details::CreateTimerfd()),
      channel_(new Channel(loop, timer_fd_)),
      calling_expired_timers_(false) {

    channel_->set_read_callback(std::bind(&TimerQueue::HandleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    channel_->EnableReading();
}

TimerQueue::~TimerQueue() {
    channel_->DisableAll();
    channel_->RemoveFromLoop();
    ::close(timer_fd_);
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when,
                             double interval) {

    Timer *timer = new Timer(std::move(cb), when, interval);
    loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::Cancel(TimerId timerId) {
    loop_->RunInLoop(std::bind(&TimerQueue::CancelInLoop, this, timerId));
}

void TimerQueue::AddTimerInLoop(Timer *timer) {
    loop_->AssertInLoopThread();
    bool earliest_changed = Insert(timer);

    if (earliest_changed) {
        details::ResetTimerfd(timer_fd_, timer->expiration());
    }
}

bool TimerQueue::Insert(Timer *timer) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());

    bool earliest_changed = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliest_changed = true;
    }
    {
        std::pair<TimerList::iterator, bool> result =
            timers_.insert(Entry(when, timer));
        assert(result.second);
        (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result =
            active_timers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void)result;
    }

    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

void TimerQueue::CancelInLoop(TimerId timer_id) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timer_id.timer_, timer_id.sequence_);
    ActiveTimerSet::iterator it = active_timers_.find(timer);
    if (it != active_timers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void)n;
        delete it->first; // FIXME: no delete please
        active_timers_.erase(it);
    } else if (calling_expired_timers_) {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead() {
    loop_->AssertInLoopThread();
    Timestamp now(Timestamp::Now());
    details::ReadTimerfd(timer_fd_, now);

    std::vector<Entry> expired = GetExpired(now);

    calling_expired_timers_ = true;
    canceling_timers_.clear();
    // safe to callback outside critical section
    for (const Entry &it : expired) {
        it.second->Run();
    }
    calling_expired_timers_ = false;

    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
    assert(timers_.size() == active_timers_.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry &it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1);
        (void)n;
    }

    assert(timers_.size() == active_timers_.size());
    return expired;
}

void TimerQueue::Reset(const std::vector<TimerQueue::Entry> &expired,
                       Timestamp now) {
    Timestamp next_expire;

    for (const Entry &it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat() &&
            canceling_timers_.find(timer) == canceling_timers_.end()) {
            it.second->Restart(now);
            Insert(it.second);
        } else {
            // FIXME move to a free list
            delete it.second; // FIXME: no delete please
        }
    }

    if (!timers_.empty()) {
        next_expire = timers_.begin()->second->expiration();
    }

    if (next_expire.Valid()) {
        details::ResetTimerfd(timer_fd_, next_expire);
    }
}

} // namespace event_loop
} // namespace muduo
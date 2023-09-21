#include "event_loop.h"
#include "channel.h"
#include "import_log.h"
#include "poller.h"
#include "timer.h"
#include "timer_queue.h"

#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace muduo {
namespace event_loop {

thread_local EventLoop *t_loop_in_this_thread = nullptr;

int CreateEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : thread_id_(this_thread::tid()),
      looping_(false),
      event_handling_(false),
      quit_(false),
      calling_pending_functors_(false),
      poller_(new EpollPoller(this)),
      wakeup_fd_(CreateEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      timer_queue_(new TimerQueue(this)) {
    LOG_DEBUG << "EventLoop created " << this << " in thread " << thread_id_;
    if (t_loop_in_this_thread) {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << thread_id_;
    } else {
        t_loop_in_this_thread = this;
    }
    wakeup_channel_->set_read_callback(
        std::bind(&EventLoop::WakeUpEventRead, this, std::placeholders::_1));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << thread_id_
              << " destructs in thread " << this_thread::tid();
    wakeup_channel_->DisableAll();
    wakeup_channel_->RemoveFromLoop();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::Loop() {
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;
    quit_ = false; // FIXME: what if someone calls quit() before loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_) {
        active_channels_.clear();
        poll_timestamp_ = poller_->Poll(10000, &active_channels_);

        event_handling_ = true;
        // empty channels if timeout
        for (Channel *channel : active_channels_) {
            current_channel_ = channel;
            current_channel_->HandleEvent(poll_timestamp_);
        }
        current_channel_ = nullptr;
        event_handling_ = false;

        CallPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::Quit() {
    quit_ = true;
    if (IsInLoopThread()) {
        Wakeup();
    }
}

void EventLoop::RunInLoop(Functor cb) {
    if (IsInLoopThread()) {
        cb();
    } else {
        QueueInLoop(std::move(cb));
    }
}

void EventLoop::QueueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(pending_functors_mutex_);
        pending_functors_.push_back(std::move(cb));
    }

    if (!IsInLoopThread() || calling_pending_functors_) {
        Wakeup();
    }
}

TimerId EventLoop::RunAt(Timestamp time, TimerCallback cb) {
    return timer_queue_->AddTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::RunAfter(double delay, TimerCallback cb) {
    auto now = Timestamp::Now();
    Timestamp time = now + delay;
    return RunAt(time, std::move(cb));
}

TimerId EventLoop::RunEvery(double interval, TimerCallback cb) {
    return RunEveryAfter(interval, interval, std::move(cb));
}

TimerId EventLoop::RunEveryAfter(double interval, double delay,
                                 TimerCallback cb) {
    auto now = Timestamp::Now();
    Timestamp time = now + delay;
    return RunEveryAt(interval, time, std::move(cb));
}

TimerId EventLoop::RunEveryAt(double interval, Timestamp time,
                              TimerCallback cb) {
    return timer_queue_->AddTimer(std::move(cb), time, interval);
}

void EventLoop::Cancel(TimerId timer_id) { timer_queue_->Cancel(timer_id); }

void EventLoop::UpdateChannel(Channel *channel) {

    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel) {
    AssertInLoopThread();
    // TODO:
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel *channel) {
    AssertInLoopThread();
    return poller_->HasChannel(channel);
}

void EventLoop::Wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::Wakeup() writes " << n
                  << " bytes instead of 8";
    }
}

void EventLoop::WakeUpEventRead(Timestamp) {
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::WakeUpEventRead() reads " << n
                  << " bytes instead of 8";
    }
}

EventLoop *EventLoop::GetEventLoopOfThisThread() {
    return t_loop_in_this_thread;
}

void EventLoop::AbortNotInLoopThread() {
    LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
              << " was created in thread_id_ = " << thread_id_
              << ", current thread id = " << this_thread::tid();
}

void EventLoop::CallPendingFunctors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        std::lock_guard<std::mutex> lock(pending_functors_mutex_);
        functors.swap(pending_functors_);
    }

    for (const Functor &functor : functors) {
        functor();
    }
    calling_pending_functors_ = false;
}

} // namespace event_loop
} // namespace muduo

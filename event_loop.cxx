#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "timer.h"
#include <assert.h>

namespace muduo {
namespace event_loop {

EventLoop::EventLoop() : poller_(new EpollPoller(this)) {}

EventLoop::~EventLoop() {}

void EventLoop::Loop() {
    quit_ = false; // FIXME: what if someone calls quit() before loop() ?

    while (!quit_) {
        active_channels_.clear();
        poll_timestamp_ = poller_->Poll(10000, &active_channels_);

        // empty channels for timeout
        for (Channel *channel : active_channels_) {
            current_channel_ = channel;
            current_channel_->HandleEvent(poll_timestamp_);
        }
        current_channel_ = nullptr;
    }
}

void EventLoop::Quit() { quit_ = true; }

void EventLoop::RunInLoop(Functor cb) {
    throw std::runtime_error("RunInLoop is not implemented");
}

void EventLoop::QueueInLoop(Functor cb) {
    throw std::runtime_error("QueueInLoop is not implemented");
}

std::shared_ptr<Timer> EventLoop::RunAt(Timestamp time, TimerCallback cb) {
    return RunEveryAt(0, time, std::move(cb));
}

std::shared_ptr<Timer> EventLoop::RunAfter(double delay, TimerCallback cb) {
    return RunEveryAfter(0, delay, std::move(cb));
}

std::shared_ptr<Timer> EventLoop::RunEvery(double interval, TimerCallback cb) {
    return RunEveryAfter(interval, 0, std::move(cb));
}

std::shared_ptr<Timer> EventLoop::RunEveryAfter(double interval, double delay,
                                                TimerCallback cb) {
    std::shared_ptr<Timer> timer(
        new Timer(this, std::move(cb), delay, interval));
    timers_.insert(std::make_pair(timer->fd(), timer));
    return timer;
}

std::shared_ptr<Timer> EventLoop::RunEveryAt(double interval, Timestamp time,
                                             TimerCallback cb) {
    std::shared_ptr<Timer> timer(
        new Timer(this, std::move(cb), time, interval));
    timers_.insert(std::make_pair(timer->fd(), timer));
    return timer;
}

void EventLoop::RemoveTimer(int timer_fd) { timers_.erase(timer_fd); }

std::size_t EventLoop::TimerCount() { return timers_.size(); }

void EventLoop::UpdateChannel(Channel *channel) {
    poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel) {
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel *channel) {
    return poller_->HasChannel(channel);
}

} // namespace event_loop
} // namespace muduo

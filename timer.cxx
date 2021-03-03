#include "timer.h"
#include "channel.h"
#include "event_loop.h"
#include "timespec.h"
#include "timestamp.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>

namespace muduo {
namespace event_loop {

Timer::Timer(EventLoop *eventloop, TimerCallback cb, Timestamp when,
             double interval_seconds)
    : eventloop_(eventloop),
      cb_(std::move(cb)),
      timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {

    double delay = when - Timestamp::Now();
    InitTimer(delay, interval_seconds);
}

Timer::Timer(EventLoop *eventloop, TimerCallback cb, double delay_seconds,
             double interval_seconds)
    : eventloop_(eventloop),
      cb_(std::move(cb)),
      timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {

    InitTimer(delay_seconds, interval_seconds);
}

void Timer::InitTimer(double delay_seconds, double interval_seconds) {
    struct itimerspec new_value;
    set_timespec_zero(new_value.it_value);
    set_timespec_zero(new_value.it_interval);

    if (delay_seconds > 0) {
        new_value.it_value = doulbe_to_timespec(delay_seconds);
    }
    if (interval_seconds > 0) {
        new_value.it_interval = doulbe_to_timespec(interval_seconds);
    }

    ::timerfd_settime(timer_fd_, 0, &new_value, nullptr);

    timer_channel_.reset(new Channel(eventloop_, timer_fd_));
    timer_channel_->EnableNonblockReading();
    timer_channel_->set_read_callback(std::bind(&Timer::ReadTimer, this));
}

Timer::~Timer() {
    Stop();
    close(timer_fd_);
}

void Timer::ReadTimer() {
    uint64_t exp = 0;
    read(timer_fd_, &exp, sizeof(exp));

    cb_();

    if (Expired()) {
        eventloop_->RemoveTimer(timer_fd_);
    }
}

void Timer::Stop() {
    struct itimerspec new_value;
    memset(&new_value, 0, sizeof new_value);

    ::timerfd_settime(timer_fd_, 0, &new_value, nullptr);
    timer_channel_->DisableAll();
    timer_channel_->RemoveFromLoop();
    timer_channel_.reset();
}

bool Timer::Expired() {
    struct itimerspec value;
    timerfd_gettime(timer_fd_, &value);
    return timespec_is_zero(value.it_value);
}

void Timer::Postpone(double seconds) {
    struct itimerspec value;
    timerfd_gettime(timer_fd_, &value);
    if (timespec_is_zero(value.it_value)) {
        return;
    }

    if (seconds > 0) {
        struct itimerspec new_value;
        set_timespec_zero(new_value.it_value);
        set_timespec_zero(new_value.it_interval);

        new_value.it_value = doulbe_to_timespec(seconds);
        new_value.it_interval = value.it_interval;

        ::timerfd_settime(timer_fd_, 0, &new_value, nullptr);
    }
}

void Timer::PostponeAfter(double seconds) {
    struct itimerspec value;
    timerfd_gettime(timer_fd_, &value);
    if (timespec_is_zero(value.it_value)) {
        return;
    }

    if (seconds > 0) {
        struct itimerspec new_value;
        set_timespec_zero(new_value.it_value);
        set_timespec_zero(new_value.it_interval);

        new_value.it_value = value.it_value + doulbe_to_timespec(seconds);
        new_value.it_interval = value.it_interval;

        ::timerfd_settime(timer_fd_, 0, &new_value, nullptr);
    }
}

} // namespace event_loop
} // namespace muduo

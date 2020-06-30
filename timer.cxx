#include "timer.h"
#include "channel.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace muduo {
namespace event_loop {

Timer::Timer(EventLoop *eventloop, TimerCallback cb, int interval_ms)
    : eventloop_(eventloop),
      cb_(std::move(cb)),
      timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof new_value);
    memset(&old_value, 0, sizeof old_value);

    new_value.it_interval.tv_sec = interval_ms / 1000;
    new_value.it_interval.tv_nsec = interval_ms % 1000 * 1000000;

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 1000000;

    int ret = ::timerfd_settime(timer_fd_, 0, &new_value, &old_value);
    (void)ret;

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
}

void Timer::Stop() {
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof new_value);
    memset(&old_value, 0, sizeof old_value);

    int ret = ::timerfd_settime(timer_fd_, 0, &new_value, &old_value);
    (void)ret;
    timer_channel_->DisableAll();
}

} // namespace event_loop
} // namespace muduo

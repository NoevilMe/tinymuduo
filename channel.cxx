#include "channel.h"
#include "event_loop.h"

#include <sys/epoll.h>

namespace muduo {
namespace event_loop {

constexpr auto kEventNone = 0;
constexpr auto kEventRead = EPOLLIN | EPOLLPRI;
constexpr auto kEventWrite = EPOLLOUT;
constexpr auto kEventEdgeTrigger = EPOLLET;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      tied_(false),
      event_handling_(false),
      events_(kEventNone),
      state_(kChannelStateNone) {}

Channel::~Channel() {}

void Channel::EnableEdgeTrigger() {
    events_ |= kEventEdgeTrigger;
    UpdateInLoop();
}

void Channel::DisableEdgeTrigger() {
    events_ &= ~kEventEdgeTrigger;
    UpdateInLoop();
}

void Channel::EnableNonblockReading() {
    events_ |= kEventEdgeTrigger | kEventRead;
    UpdateInLoop();
}

void Channel::EnableReading() {
    events_ |= kEventRead;
    UpdateInLoop();
}

void Channel::DisableReading() {
    events_ &= ~kEventRead;
    UpdateInLoop();
}

void Channel::EnableWriting() {
    events_ |= kEventWrite;
    UpdateInLoop();
}

void Channel::DisableWriting() {
    events_ &= ~kEventWrite;
    UpdateInLoop();
}

void Channel::DisableAll() {
    events_ = kEventNone;
    UpdateInLoop();
}

void Channel::HandleEvent(Timestamp ts) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            HandleEventWithGuard(ts);
        }
    } else {
        HandleEventWithGuard(ts);
    }
}

void Channel::HandleEventWithGuard(Timestamp ts) {
    event_handling_ = true;

    if ((poll_events_ & EPOLLHUP) && !(poll_events_ & EPOLLIN)) {
        if (close_cb_)
            close_cb_();
    }

    if (poll_events_ & EPOLLERR) {
        if (error_cb_)
            error_cb_();
    }

    if (poll_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_cb_)
            read_cb_(ts);
    }

    if (poll_events_ & EPOLLOUT) {
        if (write_cb_)
            write_cb_();
    }

    event_handling_ = false;
}

bool Channel::IsReading() const { return events_ & kEventRead; }

bool Channel::IsWriting() const { return events_ & kEventWrite; }

bool Channel::IsNoneEvent() const { return events_ == kEventNone; }

void Channel::Tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::UpdateInLoop() { loop_->UpdateChannel(this); }

void Channel::RemoveFromLoop() { loop_->RemoveChannel(this); }

} // namespace event_loop
} // namespace muduo

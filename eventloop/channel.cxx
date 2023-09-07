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
      added_to_loop_(false),
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

bool Channel::IsReading() const { return events_ & kEventRead; }

bool Channel::IsWriting() const { return events_ & kEventWrite; }

bool Channel::IsNoneEvent() const { return events_ == kEventNone; }

void Channel::HandleEvent(Timestamp ts) {
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) {
            HandleEventWithGuard(ts);
        }
    } else {
        HandleEventWithGuard(ts);
    }
}

void Channel::HandleEventWithGuard(Timestamp ts) {
    event_handling_ = true;

    // EPOLLIN： 表示对应的文件描述符可以读；
    // EPOLLHUP： 表示对应的文件描述符被挂断；
    // 当对方读写端都关闭，我方触发EPOLLHUP事件
    if ((poll_events_ & EPOLLHUP) && !(poll_events_ & EPOLLIN)) {
        if (close_cb_)
            close_cb_();
    }

    // EPOLLERR： 表示对应的文件描述符发生错误；
    if (poll_events_ & EPOLLERR) {
        if (error_cb_)
            error_cb_();
    }

    // EPOLLIN： 表示对应的文件描述符可以读；
    // EPOLLPRI： 表示对应的文件描述符有紧急的数据可读
    // 当对方关闭写端时shutdown(SHUT_WR)，我方触发EPOLLRDHUP事件。
    if (poll_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_cb_)
            read_cb_(ts);
    }

    // EPOLLOUT： 表示对应的文件描述符可以写
    if (poll_events_ & EPOLLOUT) {
        if (write_cb_)
            write_cb_();
    }

    event_handling_ = false;
}

void Channel::Tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::UpdateInLoop() {
    added_to_loop_ = true;
    loop_->UpdateChannel(this);
}

void Channel::RemoveFromLoop() {
    loop_->RemoveChannel(this);
    added_to_loop_ = false;
}

} // namespace event_loop
} // namespace muduo

#include "poller.h"
#include "channel.h"
#include "timestamp.h"

#include <chrono>
#include <cstring>
#include <sys/poll.h>
#include <unistd.h>

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace muduo {
namespace event_loop {

constexpr int kEpollMaxEvents = 16;

Poller::Poller(EventLoop *loop) : loop_(loop) {}

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop),
      epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kEpollMaxEvents) {}

EpollPoller::~EpollPoller() {
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

Timestamp EpollPoller::Poll(int timeout, ChannelList *active_channels) {
    int numEvents = ::epoll_wait(epoll_fd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), timeout);
    int savedErrno = errno;
    Timestamp now = Timestamp::Now();

    if (numEvents > 0) {
        for (int i = 0; i < numEvents; ++i) {
            Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
            channel->set_poll_events(events_[i].events);
            active_channels->push_back(channel);
        }

        if ((size_t)numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        ;
    } else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR) {
            errno = savedErrno;
        }
    }
    return now;
}

void EpollPoller::UpdateChannel(Channel *channel) {
    auto state = channel->state();
    if (state == kChannelStateNone || state == kChannelStateDisable) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (state == kChannelStateNone) {
            channel_map_[fd] = channel;
        }

        channel->set_state(kChannelStateEnable);
        Update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        if (channel->IsNoneEvent()) {
            Update(EPOLL_CTL_DEL, channel);
            channel->set_state(kChannelStateDisable);
        } else {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::Update(int operation, Channel *channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
        ; // error
    }
}

void EpollPoller::RemoveChannel(Channel *channel) {
    int fd = channel->fd();
    size_t n = channel_map_.erase(fd);
    (void)n;

    auto state = channel->state();
    if (state == kChannelStateEnable) {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->set_state(kChannelStateNone);
}

bool EpollPoller::HasChannel(Channel *channel) {
    return channel_map_.find(channel->fd()) != channel_map_.end();
}

} // namespace event_loop
} // namespace muduo

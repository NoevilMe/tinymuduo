#include "event_loop.h"
#include "channel.h"
#include "poller.h"
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

        for (Channel *channel : active_channels_) {
            current_channel_ = channel;
            current_channel_->HandleEvent(poll_timestamp_);
        }
        current_channel_ = nullptr;
    }
}

void EventLoop::RunInLoop(Functor cb) {}

void EventLoop::QueueInLoop(Functor cb) {}

void EventLoop::UpdateChannel(Channel *channel) {
    poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel) {
    poller_->RemoveChannel(channel);
}

} // namespace event_loop
} // namespace muduo

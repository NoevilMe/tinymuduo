#ifndef __MUDUO_CHANNEL_H_
#define __MUDUO_CHANNEL_H_

#include "callback.h"
#include "noncopyable.h"

#include <memory>

namespace muduo {
namespace event_loop {

class EventLoop;

// channel state in poller
enum ChannelState {
    kChannelStateNone = 0,
    kChannelStateEnable,
    kChannelStateDisable
};

class Channel : public Noncopyable {
public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    void set_read_callback(ReadEventCallback cb) { read_cb_ = std::move(cb); }
    void set_write_callback(EventCallback cb) { write_cb_ = std::move(cb); }
    void set_close_callback(EventCallback cb) { close_cb_ = std::move(cb); }
    void set_error_callback(EventCallback cb) { error_cb_ = std::move(cb); }

    int fd() const { return fd_; }

    ChannelState state() const { return state_; }

    void set_state(ChannelState s) { state_ = s; }

    int events() const { return events_; }

    void set_poll_events(int ev) { poll_events_ = ev; }

    bool IsNoneEvent() const;

    /// Tie this channel to the owner object managed by shared_ptr,
    /// prevent the owner object being destroyed in handleEvent.
    void Tie(const std::shared_ptr<void> &);

    // for Non-blocking fd
    void EnableEdgeTrigger();
    void DisableEdgeTrigger();

    void EnableNonblockReading();

    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();
    void DisableAll();
    bool IsReading() const;
    bool IsWriting() const;

    void RemoveFromLoop();

    void HandleEvent(Timestamp ts);

private:
    void UpdateInLoop();
    void HandleEventWithGuard(Timestamp receiveTime);

    EventLoop *loop_;
    int fd_;

    ChannelState state_;
    // watching events
    int events_;
    // occured events returned by poller
    int poll_events_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool event_handling_;
    bool added_to_loop_;

    ReadEventCallback read_cb_;
    EventCallback write_cb_;
    EventCallback close_cb_;
    EventCallback error_cb_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_CHANNEL_H_ */

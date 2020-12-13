#ifndef __MUDUO_CHANNEL_H_
#define __MUDUO_CHANNEL_H_

#include "callback.h"
#include "noncopyable.h"

namespace muduo {
namespace event_loop {

class EventLoop;

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

    bool IsNoneEvent() const;

    void SetPollEvents(int ev) { poll_events_ = ev; }

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

    EventLoop *loop_;
    int fd_;

    int events_;
    ChannelState state_;
    int poll_events_;

    ReadEventCallback read_cb_;
    EventCallback write_cb_;
    EventCallback close_cb_;
    EventCallback error_cb_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_CHANNEL_H_ */

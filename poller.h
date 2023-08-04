#ifndef __MUDUO_POLLER_H_
#define __MUDUO_POLLER_H_

#include "callback.h"
#include "noncopyable.h"

#include <map>
#include <sys/epoll.h>
#include <vector>

namespace muduo {
namespace event_loop {

class Channel;
class EventLoop;

class Poller : public Noncopyable {
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual Timestamp Poll(int timeout, ChannelList *active_channels) = 0;

    virtual void UpdateChannel(Channel *channel) = 0;
    virtual void RemoveChannel(Channel *channel) = 0;
    virtual bool HasChannel(Channel *channel) = 0;

protected:
    using ChannelMap = std::map<int, Channel *>;
    ChannelMap channel_map_;

private:
    EventLoop *loop_;
};

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller();

    Timestamp Poll(int timeout, ChannelList *active_channels) override;

    void UpdateChannel(Channel *channel) override;
    void RemoveChannel(Channel *channel) override;
    bool HasChannel(Channel *channel) override;

private:
    void Update(int operation, Channel *channel);

    int epoll_fd_;
    std::vector<struct epoll_event> events_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_POLLER_H_ */

#ifndef __EVENT_LOOP_H_
#define __EVENT_LOOP_H_

#include "event_base.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace muduo {
namespace event_loop {

class Channel;
class Poller;

class EventLoop {
public:
    using ChannelList = std::vector<Channel *>;

    EventLoop();
    ~EventLoop();

    void Loop();

    void RunInLoop(Functor cb);
    void QueueInLoop(Functor cb);

    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);

private:
    std::mutex pending_functors_mutex_;
    std::vector<Functor> pending_functors_;

    std::unique_ptr<Poller> poller_;
    bool quit_;
    ChannelList active_channels_;
    Channel *current_channel_;
    Timestamp poll_timestamp_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __EVENT_LOOP_H_ */

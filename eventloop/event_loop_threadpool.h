#ifndef __MUDUO_EVENT_LOOP_THREADPOOL_H_
#define __MUDUO_EVENT_LOOP_THREADPOOL_H_

#include "noncopyable.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace muduo {
namespace event_loop {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *base_loop, const std::string &name);
    ~EventLoopThreadPool();

    void SetThreadNum(int threads) { num_threads_ = threads; }
    void Start(const ThreadInitCallback &cb = ThreadInitCallback());

    // valid after calling start()
    /// round-robin
    EventLoop *GetNextLoop();

    /// with the same hash code, it will always return the same EventLoop
    EventLoop *GetLoopForHash(size_t hashCode);

    // std::vector<EventLoop *> GetAllLoops();

    bool started() const { return started_; }

    const std::string &name() const { return name_; }

private:
    EventLoop *base_loop_;
    std::string name_;
    bool started_;
    int num_threads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_EVENT_LOOP_THREADPOOL_H_ */

#include "event_loop_threadpool.h"
#include "event_loop.h"
#include "event_loop_thread.h"

#include <assert.h>

namespace muduo {
namespace event_loop {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop,
                                         const std::string &name)
    : base_loop_(base_loop),
      name_(name),
      started_(false),
      num_threads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start(const ThreadInitCallback &cb) {
    assert(!started_);
    base_loop_->AssertInLoopThread();

    started_ = true;

    for (int i = 0; i < num_threads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.data(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->StartLoop());
    }

    if (num_threads_ == 0 && cb) {
        cb(base_loop_);
    }
}

EventLoop *EventLoopThreadPool::GetNextLoop() {
    base_loop_->AssertInLoopThread();
    assert(started_);
    EventLoop *loop = base_loop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if ((size_t)next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::GetLoopForHash(size_t hashCode) {
    return nullptr;
}
} // namespace event_loop
} // namespace muduo
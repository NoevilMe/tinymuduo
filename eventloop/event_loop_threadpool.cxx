#include "event_loop_threadpool.h"
#include "event_loop.h"
#include "event_loop_thread.h"

namespace muduo {
namespace event_loop {
EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop,
                                         const std::string &name)
    : base_loop_(base_loop), name_(name) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start(const ThreadInitCallback &cb) {}

EventLoop *EventLoopThreadPool::GetNextLoop() { return nullptr; }

EventLoop *EventLoopThreadPool::GetLoopForHash(size_t hashCode) {
    return nullptr;
}
} // namespace event_loop
} // namespace muduo
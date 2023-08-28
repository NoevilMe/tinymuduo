#include "event_loop_thread.h"
#include "event_loop.h"

namespace muduo {
namespace event_loop {

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(nullptr),
      exiting_(false),

      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_) {
        loop_->Quit();
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

EventLoop *EventLoopThread::StartLoop() {
    thread_ = std::thread(&EventLoopThread::ThreadFunc, this);

    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
        cond_.wait(lock);
    }

    return loop_;
}

void EventLoopThread::ThreadFunc() {
    EventLoop loop;

    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.Loop();
    // assert(exiting_);
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

} // namespace event_loop
} // namespace muduo
#ifndef __MUDUO_EVENT_LOOP_THREAD_H_
#define __MUDUO_EVENT_LOOP_THREAD_H_

#include "noncopyable.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace muduo {
namespace event_loop {

class EventLoop;

class EventLoopThread : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *StartLoop();

private:
    void ThreadFunc();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_EVENT_LOOP_THREAD_H_ */

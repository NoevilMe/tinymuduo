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

#endif /* CA48AE7F_2E6B_4B1E_8AF7_5FAB83B89F70 */

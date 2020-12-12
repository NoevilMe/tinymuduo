#ifndef __MUDUO_CALLBACK_H_
#define __MUDUO_CALLBACK_H_

#include <functional>

namespace muduo {
namespace event_loop {

class Timestamp;

using Functor = std::function<void()>;

using EventCallback = std::function<void()>;
using ReadEventCallback = std::function<void(Timestamp)>;

using TimerCallback = std::function<void()>;


} // namespace event_loop
} // namespace muduo

#endif /* __EVENT_BASE_H_ */

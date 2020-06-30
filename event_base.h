#ifndef __EVENT_BASE_H_
#define __EVENT_BASE_H_

#include <functional>

namespace muduo {
namespace event_loop {

using Functor = std::function<void()>;
using Timestamp = long long;

using EventCallback = std::function<void()>;
using ReadEventCallback = std::function<void(Timestamp)>;

using TimerCallback = std::function<void()>;

} // namespace event_loop
} // namespace muduo

#endif /* __EVENT_BASE_H_ */

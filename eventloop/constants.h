#ifndef __MUDUO_CONSTANTS_H_
#define __MUDUO_CONSTANTS_H_

#include <cstdint>

namespace muduo {
namespace event_loop {

constexpr int64_t kMilliSecondsPerSecond = 1000;
constexpr int64_t kMicroSecondsPerSecond = 1000000;
constexpr int64_t kNanoSecondsPerSecond = 1000000000;
constexpr int64_t kNanoSecondsPerMilliSecond = 1000000;
constexpr int64_t kNanoSecondsPerMicroSecond = 1000;

constexpr int64_t kSecondsPerHour = 3600;

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_CONSTANTS_H_ */

#ifndef __MUDUO_TIMESPEC_H_
#define __MUDUO_TIMESPEC_H_

#include <cstdint>
#include <ctime>

namespace muduo {
namespace event_loop {

struct timespec operator-(const struct timespec &end,
                          const struct timespec &start);

struct timespec operator+(const struct timespec &start,
                          const struct timespec &post);

int64_t timespec_to_int64(const struct timespec &t);

struct timespec int64_to_timespec(int64_t i);

struct timespec doulbe_to_timespec(double s);

void set_timespec_zero(struct timespec &t);

bool timespec_is_zero(const struct timespec &t);

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMESPEC_H_ */

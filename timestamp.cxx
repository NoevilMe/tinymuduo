#include "timestamp.h"

namespace muduo {
namespace event_loop {

int64_t timespec_to_int64(const struct timespec &t);
struct timespec int64_to_timespec(int64_t i);

Timestamp::Timestamp() : nanoseconds_since_epoch_(0) {}

Timestamp::~Timestamp() {}

struct timespec Timestamp::TimespecNow() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

Timestamp Timestamp::Now() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return FromTimespec(ts);
}

Timestamp Timestamp::FromMicroSecondsSinceEpoch(int64_t microseconds) {
    return Timestamp(microseconds * 1000);
}

Timestamp Timestamp::FromMilliSecondsSinceEpoch(int64_t milliseconds) {
    return Timestamp(milliseconds * 1000000);
}

Timestamp Timestamp::FromUTC(time_t t) { return FromUTC(t, 0); }

Timestamp Timestamp::FromUTC(time_t t, int nanoseconds) {
    return Timestamp(static_cast<int64_t>(t) * kNanoSecondsPerSecond +
                     nanoseconds);
}

Timestamp Timestamp::FromTimespec(const struct timespec &ts) {
    return Timestamp(static_cast<int64_t>(ts.tv_sec) * kNanoSecondsPerSecond +
                     ts.tv_nsec);
}

} // namespace event_loop
} // namespace muduo
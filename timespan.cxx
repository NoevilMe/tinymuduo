#include "timespan.h"
#include "constants.h"

#include <cstdint>
#include <cstring>

namespace muduo {
namespace event_loop {
Timespan::Timespan(bool start) : start_time_({0, 0}) {
    if (start) {
        Timespan::SetTimespecNow(start_time_);
    }
}

Timespan::~Timespan() {}

void Timespan::Reset() { Timespan::SetTimespecNow(start_time_); }

struct timespec Timespan::Elapsed() {
    if (start_time_.tv_nsec == 0 && start_time_.tv_sec == 0) {
        return timespec({0, 0});
    } else {
        struct timespec now;
        Timespan::SetTimespecNow(now);
        return now - start_time_;
    }
}

void Timespan::SetTimespecNow(struct timespec &t) {
    clock_gettime(CLOCK_MONOTONIC, &t);
}

struct timespec operator-(const struct timespec &start,
                          const struct timespec &end) {
    int64_t si = timespec_to_int64(start);
    int64_t ei = timespec_to_int64(end);
    if (si < ei) {
        return timespec({0, 0});
    } else {
        return int64_to_timespec(ei - si);
    }
}

struct timespec int64_to_timespec(int64_t i) {
    struct timespec t;
    t.tv_sec = i / kNanoSecondsPerSecond;
    t.tv_nsec = i % kNanoSecondsPerSecond;
    return t;
}

struct timespec doulbe_to_timespec(double s) {
    struct timespec t;
    t.tv_sec = static_cast<time_t>(s);
    t.tv_nsec = (s - (long)s) * kNanoSecondsPerSecond;

    return t;
}

void set_timespec_zero(struct timespec &t) {
    // t.tv_sec = 0;
    // t.tv_nsec = 0;
    memset(&t, 0, sizeof t);
}

bool timespec_is_zero(const struct timespec &t) {
    return t.tv_sec == 0 && t.tv_nsec == 0;
}

int64_t timespec_to_int64(const struct timespec &t) {
    return int64_t(t.tv_sec) * kNanoSecondsPerSecond + t.tv_nsec;
}

} // namespace event_loop
} // namespace muduo

#include "timespan.h"
#include "constants.h"
#include "timespec.h"

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
    if (timespec_is_zero(start_time_)) {
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


} // namespace event_loop
} // namespace muduo

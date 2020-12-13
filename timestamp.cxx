#include "timestamp.h"

#include <iomanip>
#include <sstream>

namespace muduo {
namespace event_loop {

Timestamp::Timestamp() : ts_({0, 0}) {}

Timestamp::Timestamp(int64_t nanoseconds)
    : ts_({static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond),
           static_cast<long>(nanoseconds % kNanoSecondsPerSecond)}) {}

Timestamp::Timestamp(const struct timespec &ts) : ts_(ts) {}

Timestamp::~Timestamp() {}

bool Timestamp::valid() const { return ts_.tv_sec > 0 && ts_.tv_nsec > 0; }

int64_t Timestamp::nanoseconds_since_epoch() const {
    return static_cast<int64_t>(ts_.tv_sec) * kNanoSecondsPerSecond +
           ts_.tv_nsec;
}

int64_t Timestamp::microseconds_since_epoch() const {
    return static_cast<int64_t>(ts_.tv_sec) * kMicroSecondsPerSecond +
           ts_.tv_nsec / kNanoSecondsPerMicroSecond;
}

int64_t Timestamp::milliseconds_since_epoch() const {
    return static_cast<int64_t>(ts_.tv_sec) * kMilliSecondsPerSecond +
           ts_.tv_nsec / kNanoSecondsPerMilliSecond;
}

time_t Timestamp::seconds_since_epoch() const { return ts_.tv_sec; }

std::string Timestamp::ToString() const {
    char buf[32] = {0};

    snprintf(buf, sizeof(buf), "%ld.%09ld", ts_.tv_sec, ts_.tv_nsec);
    return buf;
}

std::string Timestamp::ToFormattedString() const {
    std::tm *bt = localtime(&ts_.tv_sec);
    std::ostringstream oss;
    // https://en.cppreference.com/w/cpp/io/manip/put_time
    oss << std::put_time(bt, "%F %T");
    oss << "." << std::setfill('0') << std::setw(9) << ts_.tv_nsec;
    return oss.str();
}

std::string Timestamp::ToFormattedMilliSecondsString() const {
    std::tm *bt = localtime(&ts_.tv_sec);
    std::ostringstream oss;
    // https://en.cppreference.com/w/cpp/io/manip/put_time
    oss << std::put_time(bt, "%F %T");
    oss << "." << std::setfill('0') << std::setw(3)
        << ts_.tv_nsec / kNanoSecondsPerMilliSecond;
    return oss.str();
}

std::string Timestamp::ToFormattedMicroSecondsString() const {
    std::tm *bt = localtime(&ts_.tv_sec);
    std::ostringstream oss;
    // https://en.cppreference.com/w/cpp/io/manip/put_time
    oss << std::put_time(bt, "%F %T");
    oss << "." << std::setfill('0') << std::setw(6)
        << ts_.tv_nsec / kNanoSecondsPerMicroSecond;
    return oss.str();
}

struct timespec Timestamp::TimespecNow() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

Timestamp Timestamp::Now() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return Timestamp(ts);
}

Timestamp Timestamp::FromMicroSecondsSinceEpoch(int64_t microseconds) {
    return Timestamp(microseconds * kNanoSecondsPerMicroSecond);
}

Timestamp Timestamp::FromMilliSecondsSinceEpoch(int64_t milliseconds) {
    return Timestamp(milliseconds * kNanoSecondsPerMilliSecond);
}

Timestamp Timestamp::FromUTC(time_t t) { return FromUTC(t, 0); }

Timestamp Timestamp::FromUTC(time_t t, int nanoseconds) {
    struct timespec ts({t, nanoseconds});
    return Timestamp(ts);
}

} // namespace event_loop
} // namespace muduo
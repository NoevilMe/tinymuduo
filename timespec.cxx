#include "timespec.h"
#include "constants.h"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace muduo {
namespace event_loop {

struct timespec operator-(const struct timespec &end,
                          const struct timespec &start) {
    int64_t ei = timespec_to_int64(end);
    int64_t si = timespec_to_int64(start);

    if (ei < si) {
        return timespec({0, 0});
    } else {
        return int64_to_timespec(ei - si);
    }
}

bool operator<(const struct timespec &lv, const struct timespec &rv) {
    if (lv.tv_sec < rv.tv_sec) {
        return true;
    } else if (lv.tv_sec == rv.tv_sec) {
        return lv.tv_nsec < rv.tv_nsec;
    } else {
        return false;
    }
}

struct timespec operator+(const struct timespec &start,
                          const struct timespec &post) {
    int64_t si = timespec_to_int64(start);
    int64_t pi = timespec_to_int64(post);
    return int64_to_timespec(pi + si);
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

std::string FormatMilliSencond(const struct timespec &t) {
    std::tm *bt = localtime(&t.tv_sec);
    std::ostringstream oss;
    // https://en.cppreference.com/w/cpp/io/manip/put_time
    oss << std::put_time(bt, "%F %T");
    oss << "." << std::setfill('0') << std::setw(3) << t.tv_nsec / 1000000;
    return oss.str();
}

std::string FormatMicroSencond(const struct timespec &t) {
    std::tm *bt = localtime(&t.tv_sec);
    std::ostringstream oss;
    // https://en.cppreference.com/w/cpp/io/manip/put_time
    oss << std::put_time(bt, "%F %T");
    oss << "." << std::setfill('0') << std::setw(6) << t.tv_nsec / 1000;
    return oss.str();
}

} // namespace event_loop
} // namespace muduo

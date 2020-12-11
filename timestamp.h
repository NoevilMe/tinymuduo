#ifndef __MUDUO_TIMESTAMP_H_
#define __MUDUO_TIMESTAMP_H_

#include "constants.h"

#include <cstdint>
#include <ctime>

namespace muduo {
namespace event_loop {

class Timestamp {
public:
    Timestamp();
    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param nanoseconds
    explicit Timestamp(int64_t nanoseconds)
        : nanoseconds_since_epoch_(nanoseconds) {}

    ~Timestamp();

    bool valid() const { return nanoseconds_since_epoch_ > 0; }

    int64_t nanoseconds_since_epoch() const { return nanoseconds_since_epoch_; }

    int64_t microseconds_since_epoch() const {
        return nanoseconds_since_epoch_ / 1000;
    }
    int64_t milliseconds_since_epoch() const {
        return nanoseconds_since_epoch_ / 1000000;
    }

    time_t seconds_since_epoch() const {
        return static_cast<time_t>(nanoseconds_since_epoch_ /
                                   kMicroSecondsPerSecond);
    }

    // string toString() const;
    // string toFormattedString(bool showMicroseconds = true) const;

    static struct timespec TimespecNow();

    static Timestamp Now();

    static Timestamp FromMicroSecondsSinceEpoch(int64_t microseconds);

    static Timestamp FromMilliSecondsSinceEpoch(int64_t milliseconds);

    static Timestamp FromUTC(time_t t);

    static Timestamp FromUTC(time_t t, int nanoseconds);

    static Timestamp FromTimespec(const struct timespec &ts);

private:
    int64_t nanoseconds_since_epoch_;
};

inline bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.nanoseconds_since_epoch() < rhs.nanoseconds_since_epoch();
}

inline bool operator==(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.nanoseconds_since_epoch() == rhs.nanoseconds_since_epoch();
}

inline double operator-(const Timestamp &high, const Timestamp &low) {
    int64_t diff =
        high.nanoseconds_since_epoch() - low.nanoseconds_since_epoch();
    return static_cast<double>(diff) / kNanoSecondsPerSecond;
}

inline Timestamp operator+(const Timestamp &ts, double senconds) {
    int64_t delta = static_cast<int64_t>(senconds * kNanoSecondsPerSecond);
    return Timestamp(ts.nanoseconds_since_epoch() + delta);
}

} // namespace event_loop
} // namespace muduo

#endif /* __TIMER_H_ */

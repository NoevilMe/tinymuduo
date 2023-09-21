#ifndef __MUDUO_TIMESTAMP_H_
#define __MUDUO_TIMESTAMP_H_

#include "constants.h"

#include <cstdint>
#include <ctime>
#include <string>

namespace muduo {
namespace event_loop {

class Timestamp {
public:
    Timestamp();
    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param nanoseconds
    explicit Timestamp(int64_t nanoseconds);

    /// @param timespec ts
    explicit Timestamp(const struct timespec &ts);

    ~Timestamp();

    bool Valid() const;

    int64_t NanosecondsSinceEpoch() const;
    int64_t MicrosecondsSinceEpoch() const;
    int64_t MillisecondsSinceEpoch() const;
    time_t SecondsSinceEpoch() const;

    std::string ToString() const;
    std::string ToFormattedString() const;
    std::string ToFormattedMilliSecondsString() const;
    std::string ToFormattedMicroSecondsString() const;

    static struct timespec TimespecNow();

    static Timestamp Now();

    static Timestamp FromMicroSecondsSinceEpoch(int64_t microseconds);

    static Timestamp FromMilliSecondsSinceEpoch(int64_t milliseconds);

    static Timestamp FromUTC(time_t t);

    static Timestamp FromUTC(time_t t, int nanoseconds);

    static Timestamp FromTimespec(const struct timespec &ts);

    static struct timespec HowMuchTimeFromNow(Timestamp when);

    bool operator==(const Timestamp &rhs) {
        return ts_.tv_sec == rhs.ts_.tv_sec && ts_.tv_nsec == rhs.ts_.tv_nsec;
    }

private:
    struct timespec ts_;
};

inline bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.NanosecondsSinceEpoch() < rhs.NanosecondsSinceEpoch();
}

inline bool operator>(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.NanosecondsSinceEpoch() > rhs.NanosecondsSinceEpoch();
}

inline bool operator!=(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.NanosecondsSinceEpoch() != rhs.NanosecondsSinceEpoch();
}

inline double operator-(const Timestamp &high, const Timestamp &low) {
    int64_t diff = high.NanosecondsSinceEpoch() - low.NanosecondsSinceEpoch();
    return static_cast<double>(diff) / kNanoSecondsPerSecond;
}

inline Timestamp operator+(const Timestamp &ts, double senconds) {
    int64_t delta = static_cast<int64_t>(senconds * kNanoSecondsPerSecond);
    return Timestamp(ts.NanosecondsSinceEpoch() + delta);
}

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMESTAMP_H_ */

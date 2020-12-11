#ifndef __MUDUO_TIMESPAN_H_
#define __MUDUO_TIMESPAN_H_

#include <cstdint>
#include <ctime>

namespace muduo {
namespace event_loop {

class Timespan {
public:
    Timespan(bool start = true);
    ~Timespan();

    void Reset();
    struct timespec Elapsed();

    static void SetTimespecNow(struct timespec &t);

private:
    struct timespec start_time_;
};

struct timespec operator-(const struct timespec &start,
                          const struct timespec &end);

int64_t timespec_to_int64(const struct timespec &t);

struct timespec int64_to_timespec(int64_t i);

struct timespec doulbe_to_timespec(double s);

void set_timespec_zero(struct timespec &t);

bool timespec_is_zero(const struct timespec &t);

} // namespace event_loop
} // namespace muduo

#endif /* __TIMER_H_ */

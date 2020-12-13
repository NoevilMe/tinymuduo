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


} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_TIMESPAN_H_ */

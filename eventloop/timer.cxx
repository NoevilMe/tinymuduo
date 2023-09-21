#include "timer.h"
#include "channel.h"
#include "event_loop.h"
#include "timespec.h"
#include "timestamp.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>

namespace muduo {
namespace event_loop {

std::atomic_int64_t Timer::s_num(0);

void Timer::Restart(Timestamp now) {
    if (repeat_) {
        expiration_ = now + interval_;
    } else {
        expiration_ = Timestamp(); // invalid
    }
}

} // namespace event_loop
} // namespace muduo

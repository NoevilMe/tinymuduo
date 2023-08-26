#include "this_thread.h"

#include <cstdio>
#include <type_traits>

#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace muduo {
namespace this_thread {

thread_local int t_cached_tid = 0;
thread_local char t_tid_string[32] = {0};
thread_local int t_tid_string_length = 6;
thread_local const char *t_threadName = "unknown";
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

void CacheTid() {
    if (t_cached_tid == 0) {
        t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
        t_tid_string_length =
            snprintf(t_tid_string, sizeof t_tid_string, "%5d ", t_cached_tid);
    }
}

} // namespace this_thread
} // namespace muduo
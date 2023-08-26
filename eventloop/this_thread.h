#ifndef __MUDUO_THIS_THREAD_H_
#define __MUDUO_THIS_THREAD_H_

namespace muduo {
namespace this_thread {

extern thread_local int t_cached_tid;
extern thread_local char t_tid_string[32];
extern thread_local int t_tid_string_length;

void CacheTid();

inline int tid() {
    if (__builtin_expect(t_cached_tid == 0, 0)) {
        CacheTid();
    }
    return t_cached_tid;
}

inline const char *tid_string() // for logging
{
    return t_tid_string;
}

inline int tid_string_length() // for logging
{
    return t_tid_string_length;
}

} // namespace this_thread
} // namespace muduo

#endif /* __MUDUO_THIS_THREAD_H_ */

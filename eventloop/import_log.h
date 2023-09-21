#ifndef __MUDUO_EVENTLOOP_IMPORT_LOG_H_
#define __MUDUO_EVENTLOOP_IMPORT_LOG_H_

#ifdef EVENTLOOP_USE_MUDUO_LOGGER
#include "logger/logger.h"
#else
class NullStream {
public:
    template <typename T>
    NullStream &operator<<(T) {
        return *this;
    }
};

#define LOG_TRACE                                                              \
    if (false)                                                                 \
    NullStream()
#define LOG_DEBUG                                                              \
    if (false)                                                                 \
    NullStream()
#define LOG_INFO                                                               \
    if (false)                                                                 \
    NullStream()
#define LOG_WARN                                                               \
    if (false)                                                                 \
    NullStream()
#define LOG_ERROR                                                              \
    if (false)                                                                 \
    NullStream()
#define LOG_FATAL                                                              \
    if (false)                                                                 \
    NullStream()
#define LOG_SYSERR                                                             \
    if (false)                                                                 \
    NullStream()
#define LOG_SYSFATAL                                                           \
    if (false)                                                                 \
    NullStream()
#endif

#endif /* B8C792F1_57DA_4FF8_8522_A17028BBF784 */

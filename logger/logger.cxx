#include "logger.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>

#include <assert.h>
#include <syscall.h>
#include <unistd.h>

namespace muduo {

static constexpr int64_t kMicroSecondsPerSecond = 1000000;

static int64_t timestamp_microseconds_now() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

namespace localthread {
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread char t_time_micro[10];
__thread time_t t_lastsecond = -1;

__thread int t_cached_tid = 0;
__thread char t_tid_str[32] = {0};
__thread int t_tid_str_length = 0;

void CacheTid() {
    if (t_cached_tid == 0) {
        t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
        t_tid_str_length =
            snprintf(t_tid_str, sizeof t_tid_str, "%5d ", t_cached_tid);
    }
}

inline int tid() {
    if (__builtin_expect(t_cached_tid == 0, 0)) {
        CacheTid();
    }
    return t_cached_tid;
}

inline const char *tid_string() // for logging
{
    return t_tid_str;
}

inline int tid_string_length() // for logging
{
    return t_tid_str_length;
}

}; // namespace localthread

const char *strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, localthread::t_errnobuf,
                      sizeof(localthread::t_errnobuf));
}

namespace log {

/**
 * @brief 日志输出重定向
 *
 * @param msg
 * @param len
 */
void DefaultOutput(const char *msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    // FIXME check n
    (void)n;
}

void DefaultFlush() { fflush(stdout); }

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

void Logger::SetOutput(OutputFunc func) { g_output = func; }
void Logger::SetFlush(FlushFunc func) { g_flush = func; }

/**
 * @brief 全局日志等级设置
 *
 * @return Logger::LogLevel
 */
static Logger::LogLevel InitLogLevel() {
    if (::getenv("MUDUO_LOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("MUDUO_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_log_level = InitLogLevel();

const char *LogLevelName[Logger::NUM_LEVELS] = {
    "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

void Logger::set_log_level(LogLevel level) { g_log_level = level; }

// helper class for known string length at compile time
class T {
public:
    T(const char *str, unsigned len) : str_(str), len_(len) {
        // std::cout << "str_ \"" << str_ << "\",  len_ " << len_ << std::endl;
        assert(strlen(str) == len_);
    }

    const char *str_;
    const unsigned len_;
};

inline LogStream &operator<<(LogStream &s, T v) {
    s.Append(v.str_, v.len_);
    return s;
}

inline LogStream &operator<<(LogStream &s, const Logger::SourceFile &v) {
    s.Append(v.data_, v.size_);
    return s;
}

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile &file,
                   int line)
    : micro_ts_(timestamp_microseconds_now()),
      level_(level),
      line_(line),
      basename_(file) {
    FormatTime();
    localthread::tid();
    stream_ << T(localthread::tid_string(), localthread::tid_string_length());
    stream_ << T(LogLevelName[level], 6);
    if (old_errno != 0) {
        stream_ << strerror_tl(old_errno) << " (errno=" << old_errno << ")";
    }
}

void Logger::Impl::FormatTime() {
    time_t seconds = micro_ts_ / kMicroSecondsPerSecond;
    if (seconds != localthread::t_lastsecond) {
        localthread::t_lastsecond = seconds;

        std::tm *local_time = localtime(&seconds);
        // 写入此线程存储的时间buf中
        snprintf(localthread::t_time, sizeof(localthread::t_time),
                 "%4d/%02d/%02d %02d:%02d:%02d", local_time->tm_year + 1900,
                 local_time->tm_mon + 1, local_time->tm_mday,
                 local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    }

    stream_ << T(localthread::t_time, 19);

    int microseconds = static_cast<int>(micro_ts_ % kMicroSecondsPerSecond);

    snprintf(localthread::t_time_micro, sizeof(localthread::t_time_micro),
             ".%06d ", microseconds);
    stream_ << T(localthread::t_time_micro, 8);
}

void Logger::Impl::Finish() {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(INFO, 0, file, line) {
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool abort)
    : impl_(abort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    impl_.Finish();
    const LogStream::SmallBuffer &buf(stream().buffer());
    g_output(buf.data(), buf.Length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

} // namespace log
} // namespace muduo
#ifndef DAC32AFD_11D3_44B9_A23C_18BD206270B8
#define DAC32AFD_11D3_44B9_A23C_18BD206270B8

#include "log_stream.h"

#include <cstring>

namespace muduo {
namespace log {

class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LEVELS,
    };

    // SourceFile的作用是提取文件名
    class SourceFile {
    public:
        template <int N>
        SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
            const char *slash = strrchr(data_, '/'); // builtin function
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char *filename) : data_(filename) {
            /**
             * 找出data中出现/最后一次的位置，从而获取具体的文件名
             * 2022/10/26/test.log
             */
            const char *slash = strrchr(filename, '/');
            if (slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char *data_;
        int size_;
    };

    // member function
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char *func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    // 返回的LogStream对象可以继续执行<<操作符，流是可以改变的，默认是stdout
    LogStream &stream() { return impl_.stream_; }

    static LogLevel log_level();
    static void set_log_level(LogLevel level);

    // 输出函数和刷新缓冲区函数
    typedef void (*OutputFunc)(const char *msg, int len);
    typedef void (*FlushFunc)();
    /**
     * 下面两个设置为静态函数的原因：本文件结尾的宏定义可见，每一条日志都是临时创建一个Logger对象，所以每条日志结束后马上析构
     * 而且宏定义中也没有setOutput函数，因此把setOutput设置成静态函数，只需要一个地方调用了setOutput，那么所有临时对象都会
     * 输出到setOutput指定的位置
     */
    static void SetOutput(OutputFunc); // 设置日志输出位置
    static void SetFlush(FlushFunc);

private:
    // 内部类
    class Impl {
    public:
        using LogLevel = Logger::LogLevel;

        Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
        // 把时间变成字符串的形式，并apend到LogStream的buffer_中
        void FormatTime();
        // 每条日志收尾的信息apend到LogStream的buffer_中，比如文件名、行号、换行符
        void Finish();

        LogStream stream_;
        int64_t micro_ts_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };

    // Logger's member variable
    Impl impl_;
};

extern Logger::LogLevel g_log_level;

inline Logger::LogLevel Logger::log_level() { return g_log_level; }

/**
 * 当日志等级小于对应等级才会输出
 * 比如设置等级为FATAL，则logLevel等级大于DEBUG和INFO，DEBUG和INFO等级的日志就不会输出
 */
#define LOG_TRACE                                                              \
    if (muduo::log::Logger::log_level() <= muduo::log::Logger::TRACE)          \
    muduo::log::Logger(__FILE__, __LINE__, muduo::log::Logger::TRACE,          \
                       __func__)                                               \
        .stream()
#define LOG_DEBUG                                                              \
    if (muduo::log::Logger::log_level() <= muduo::log::Logger::DEBUG)          \
    muduo::log::Logger(__FILE__, __LINE__, muduo::log::Logger::DEBUG,          \
                       __func__)                                               \
        .stream()
#define LOG_INFO                                                               \
    if (muduo::log::Logger::log_level() <= muduo::log::Logger::INFO)           \
    muduo::log::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN                                                               \
    muduo::log::Logger(__FILE__, __LINE__, muduo::log::Logger::WARN).stream()
#define LOG_ERROR                                                              \
    muduo::log::Logger(__FILE__, __LINE__, muduo::log::Logger::ERROR).stream()
#define LOG_FATAL                                                              \
    muduo::log::Logger(__FILE__, __LINE__, muduo::log::Logger::FATAL).stream()
#define LOG_SYSERR muduo::log::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::log::Logger(__FILE__, __LINE__, true).stream()

} // namespace log

const char *strerror_tl(int saved_errno);

} // namespace muduo

#endif /* DAC32AFD_11D3_44B9_A23C_18BD206270B8 */

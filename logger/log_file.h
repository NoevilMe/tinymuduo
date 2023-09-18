#ifndef __MUDUO_LOG_FILE_H_
#define __MUDUO_LOG_FILE_H_

#include <memory>
#include <mutex>
#include <string>

namespace muduo {
namespace log {

/**
 * FileWriter：工具类，用于打开一个文件fp_，设置全缓冲。把buffer_中的数据写入文件中，在LogFile中用到
 */
class FileWriter {

public:
    // 构造时会把文件指针fp_的缓冲区设置为本地的buffer_
    explicit FileWriter(const std::string &fileName);
    ~FileWriter();

    // 向buffer_中添加数据
    void Append(const char *data, size_t len);

    // 刷新（相当于把buffer_中的数据写入到fp_打开的文件中）
    void Flush();

    // 返回已经写入了多少字节，以便LogFile根据写入数据量来判断是否需要滚动日志
    size_t written_bytes() const { return written_bytes_; }

private:
    size_t Write(const char *data, size_t len);

    FILE *fp_;               // 文件指针
    char buffer_[64 * 1024]; // 64KB缓冲区
    // 指示文件偏移量(指示当前已经文件(注意不是buffer_)写入多少字节)
    size_t written_bytes_;
};

/**
 * LogFile类用于向文件中写入日志信息，具有在合适时机创建日志文件以及把数据写入文件的功能
 */
class LogFile {

public:
    // threadSafe表示是否需要考虑线程安全，其的作用：同步日志需要锁，因为日志可能来自多个线程，异步日志不需要锁，
    // 因为异步日志的日志信息全部只来自异步线程（见AsyncLogging::threadFunc），无需考虑线程安全。
    // 由于默认是同步日志，因此threadSafe默认为true
    LogFile(const std::string &basename, size_t roll_size,
            bool thread_safe = true, int flush_interval = 3,
            int flush_every_n = 1024);
    ~LogFile() = default;

    // 向file_的缓冲区buffer_中继续添加日志信息
    void Append(const char *logline, int len);
    void Flush(); // 把文件缓冲区中的日志写入强制写入到文件中
    bool RollFile(); // 滚动日志，相当于创建一个新的文件，用来存储日志

private:
    // 根据当前时间生成一个文件名并返回
    static std::string GetLogFileName(const std::string &basename, time_t *now);
    void AppendUnlocked(const char *logline, int len);

    const std::string basename_;

    // 是否滚动日志（创建新的文件存储日志）的阈值，file_的写入数据超过该值就会滚动日志
    const size_t roll_size_;
    // 刷新时间间隔(s)
    const int flush_interval_;
    // 文件缓冲区（file_的buffer_）写入数据的长度没超过rollSize_(可理解为没写满)的情况下，（见append函数实现）
    // 也会在往file_的buffer_写r入的次数达到checkEveryN_时，刷新一下（fflush），即把缓冲区的数据存储到文件中。
    // 记录往buffer_添加数据的次数,超过checkEveryN_次就fflush一下
    const int flush_every_n_;

    int count_;

    std::unique_ptr<std::mutex> mutex_;
    time_t start_of_period_; // 记录最后一个日志文件是哪一天创建的（单位是秒）
    time_t last_roll_;  // 最后一次滚动日志的时间（s）
    time_t last_flush_; // 最后一次刷新的时间
    std::unique_ptr<FileWriter> file_writer_;

    const static int kRollPerSeconds_ = 60 * 60 * 24; // 一天
};

} // namespace log
} // namespace muduo
#endif /* __MUDUO_LOG_FILE_H_ */

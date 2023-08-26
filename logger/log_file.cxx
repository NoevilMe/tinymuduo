#include "log_file.h"
#include "logger.h"

#include <cstring>

namespace muduo {
namespace log {

FileWriter::FileWriter(const std::string &fileName)
    : fp_(::fopen(fileName.data(), "ae")), written_bytes_(0) {
    // ‘a’是追加，'e'表示O_CLOEXEC
    // 将fd_缓冲区设置为本地的buffer_，这样调用write的时候才会借助缓冲区，并在适当的时机写入文件
    // setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ); 设置成全缓冲
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileWriter::~FileWriter() { ::fclose(fp_); }

void FileWriter::Append(const char *data, size_t len) {
    // 记录已经写入的数据大小
    size_t written = 0;
    while (written != len) // 只要没写完就一直写
    {
        // 还需写入的数据大小
        size_t remain = len - written;
        size_t n = Write(data + written, remain); // 返回成功写如入了n字节
        if (n != remain) // 剩下的没有写完就看下是否出错，没出错就继续写
        {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "FileUtil::Append() failed %s\n",
                        strerror_tl(err));
            }
        }
        // 更新写入的数据大小
        written += n;
    }
    // 记录目前为止写入的数据大小，超过限制会滚动日志(滚动日志在LogFile中实现)
    written_bytes_ += written;
}

size_t FileWriter::Write(const char *data, size_t len) {
    /**
     * size_t fwrite(const void* buffer, size_t size, size_t count, FILE*
     * stream);
     * -- buffer:指向数据块的指针
     * -- size:每个数据的大小，单位为Byte(例如：sizeof(int)就是4)
     * -- count:数据个数
     * -- stream:文件指针
     */
    // fwrite_unlocked不是线程安全的
    return ::fwrite_unlocked(data, 1, len, fp_);
}

void FileWriter::Flush() { ::fflush(fp_); }

LogFile::LogFile(const std::string &basename, size_t roll_size,
                 bool thread_safe, int flush_interval, int flush_every_n)
    : basename_(basename),
      roll_size_(roll_size),
      flush_interval_(flush_interval),
      flush_every_n_(flush_every_n),
      count_(0),
      // 同步日志需要锁，而异步日志不需要锁
      mutex_(thread_safe ? new std::mutex : nullptr),
      start_of_period_(0),
      last_roll_(0),
      last_flush_(0) {
    RollFile();
}

void LogFile::Append(const char *logline, int len) {
    // 开启异步日志时似乎不用加锁，因为异步日志在apend的时候是将前端日志指针buffer_和后端异步日志指针bufferToWrite交换了的
    // 从构造函数可见，mutex_不为空，说明要考虑线程安全，默认是同步日志，日志可能来自多个线程，所以要锁
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        AppendUnlocked(logline, len);
    } else {
        AppendUnlocked(logline, len);
    }
}

void LogFile::AppendUnlocked(const char *logline, int len) {
    file_writer_->Append(logline, len);
    // 如果file_指向的缓冲区buffer_已经写入的数据超过了设定的滚动日志的阈值rollSize_，就开始滚动日志
    if (file_writer_->written_bytes() > roll_size_) {
        RollFile();
    } else {
        ++count_;
        if (count_ >= flush_every_n_) {
            count_ = 0;
            time_t now = ::time(NULL);
            // kRollPerSeconds_默认是1天，now / kRollPerSeconds_
            // 会自动取整，即得到从纪元
            // 开始计算到今天的天数，这个天数乘以kRollPerSeconds_就得到从纪元到今天0点的秒数
            time_t this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
            // 尽管之前创建的文件还没达到rollSize_(可理解为还没写满)，但是，如果上次创建的日志文件和当前并不是同一天，
            // 即thisPeriod !=
            // startOfPeriod_，那么还是会创建新的日志文件，简单来说就是:不管之前创建的(最开始创建的)
            // 日志文件是否写满,日志系统每天都会创建新的日志文件，
            if (this_period != start_of_period_) {
                // 如果今天没有创建日志文件，则创建一个
                RollFile();
            } else if (now - last_flush_ > flush_interval_) {
                last_flush_ = now;
                file_writer_->Flush();
            }
        }
    }
}

void LogFile::Flush() {
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        file_writer_->Flush();
    } else {
        file_writer_->Flush();
    }
}

// 滚动日志（创建一个新的文件）
// basename + time + hostname + pid + ".log"
// 会发生混动日志的情况：
// 1）日志大小超过了设定的阈值rollSize_
// 2）过了0点（注意不是每天0点准时创建，而是在append的时候判断刚才正在使用的日志文件是否是昨天/更久之前创建的，
// 只要今天还没有创建日志文件，那么就会在append的时候创建一个）
bool LogFile::RollFile() {
    time_t now = 0;
    std::string filename = GetLogFileName(basename_, &now);

    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        // 计算现在是第几天
        // now/kRollPerSeconds求出现在是第几天，再乘以秒数相当于是当前天数0点对应的秒数
        time_t this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
        start_of_period_ =
            this_period; // 更新一下startOfPeriod_，表示今天已经创建日志了
        // 让file_指向一个名为filename的文件，相当于新建了一个文件
        file_writer_.reset(new FileWriter(filename));
        return true;
    }
    return false;
}

// 根据当前时间生成一个文件名并返回
std::string LogFile::GetLogFileName(const std::string &basename, time_t *now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm);
    // 写入时间
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    filename += ".log";

    return filename;
}

} // namespace log
} // namespace muduo
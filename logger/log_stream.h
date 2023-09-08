#ifndef C5CEB474_D88A_4EC6_A948_23C2A873BC2D
#define C5CEB474_D88A_4EC6_A948_23C2A873BC2D

#include "fixed_buffer.h"
#include "noncopyable.h"
#include "string_piece.h"

namespace muduo {
namespace log {

class LogStream : Noncopyable {
public:
    using SmallBuffer = FixedBuffer<kSmallBuffer>; // 4KB大小

    void Append(const char *data, int len) { buffer_.Append(data, len); }
    const SmallBuffer &buffer() const { return buffer_; }
    void ResetBuffer() { buffer_.Reset(); }

    LogStream &operator<<(bool);

    LogStream &operator<<(short);
    LogStream &operator<<(unsigned short);

    LogStream &operator<<(int);
    LogStream &operator<<(unsigned int);

    LogStream &operator<<(long);
    LogStream &operator<<(unsigned long);
    LogStream &operator<<(long long);
    LogStream &operator<<(unsigned long long);

    LogStream &operator<<(float v);
    LogStream &operator<<(double v);

    LogStream &operator<<(char c);
    LogStream &operator<<(const void *p);
    LogStream &operator<<(const char *str);
    LogStream &operator<<(const unsigned char *str);
    LogStream &operator<<(const std::string &str);
    LogStream &operator<<(const StringPiece &v);
    LogStream &operator<<(const SmallBuffer &buf);

private:
    template <typename T>
    void convertInteger(T); // 将整数处理成字符串并添加到buffer_中

private:
    static const int kMaxNumericSize = 48;

    SmallBuffer buffer_;
};

} // namespace log
} // namespace muduo

#endif /* C5CEB474_D88A_4EC6_A948_23C2A873BC2D */

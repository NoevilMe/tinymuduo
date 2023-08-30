#include "log_stream.h"

#include <algorithm>
#include <cstring>

namespace muduo {
namespace log {

const char g_digits[] = "9876543210123456789";
const char *g_zero = g_digits + 9;

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

size_t ConvertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char *p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

LogStream &LogStream::operator<<(bool v) {
    buffer_.Append(v ? "1" : "0", 1);
    return *this;
}

LogStream &LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream &LogStream::operator<<(int v) {
    convertInteger(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned int v) {
    convertInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long v) {
    convertInteger(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long v) {
    convertInteger(v);
    return *this;
}
LogStream &LogStream::operator<<(long long v) {
    convertInteger(v);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long long v) {
    convertInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}
LogStream &LogStream::operator<<(double v) { return *this; }

LogStream &LogStream::operator<<(char v) {
    buffer_.Append(&v, 1);
    return *this;
}
LogStream &LogStream::operator<<(const void *p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.Avail() >= kMaxNumericSize) {
        char *buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = ConvertHex(buf + 2, v);
        buffer_.Add(len + 2);
    }
    return *this;
}
LogStream &LogStream::operator<<(const char *str) {
    if (str) {
        buffer_.Append(str, strlen(str));
    } else {
        buffer_.Append("(null)", 6);
    }
    return *this;
}
LogStream &LogStream::operator<<(const unsigned char *str) {
    return operator<<(reinterpret_cast<const char *>(str));
}

LogStream &LogStream::operator<<(const std::string &str) {
    buffer_.Append(str.data(), str.size());
    return *this;
}
LogStream &LogStream::operator<<(const SmallBuffer &buf) { return *this; }

// 将整数处理成字符串并添加到buffer_中
template <typename T>
void LogStream::convertInteger(T num) {
    if (buffer_.Avail() >= kMaxNumericSize) {

        char *start = buffer_.current();
        char *cur = start;
        bool negative = (num < 0); // 是否为负数

        // 从数字的末位开始转化成字符，最后在翻转字符串
        do {
            int remainder = static_cast<int>(num % 10);
            *cur++ = g_zero[remainder];
            num = num / 10;
        } while (num != 0);

        if (negative) { // 如果是负数就添加负号
            *cur++ = '-';
        }
        *cur = '\0';

        std::reverse(start, cur);
        buffer_.Add(static_cast<int>(cur - start));
    }
}

} // namespace log
} // namespace muduo
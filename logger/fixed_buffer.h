#ifndef __MUDUO_FIXED_BUFFER_H_
#define __MUDUO_FIXED_BUFFER_H_

#include "string_piece.h"

#include <cstring>
#include <string>

namespace muduo {

/**
 * 自定义的Buffer工具类，可以申请制定大小的空间的内存区域，并实现了一些接口方便
 * 从buffer中写入数据或者取出数据。
 *
 * 异步日志的就需要前端先将日志信息写入Buffer，然后后端从buffer中取出数据写入stdout
 * 或者文件，因此需要一个Buffer工具类
 */

// Buffer默认提供一下两种大小
constexpr int kSmallBuffer = 4096; //4K 用于日志行大小
constexpr int kLargeBuffer = 4096 * 1024;

// Buffer的大小（可以是任意大于0的整数，但是日志系统默认用到的是kSmallBuffer和kLargeBuffer）
template <int SIZE>
class FixedBuffer {

public:
    FixedBuffer() : cur_(data_) {}
    ~FixedBuffer() = default;

    // 像FIxedBuffer中添加数据，如果空间不够则不做任何处理
    void Append(const char *buf, size_t len) {
        if (static_cast<size_t>(Avail()) > len) {
            std::memcpy(cur_, buf, len);
            cur_ += len;
        }
    }
    // 返回FixedBuffer首地址
    const char *data() const { return data_; }
    char *current() { return cur_; }
    int Length() const { return static_cast<int>(cur_ - data_); }

    size_t Avail() const { return static_cast<size_t>(end() - cur_); }

    void Add(size_t len) { cur_ += len; }
    void Reset() { cur_ = data_; }
    void Bzero() { ::bzero(data_, sizeof(data_)); }
    std::string ToString() const { return std::string(data_, Length()); }
    StringPiece ToStringPiece() const { return StringPiece(data_, Length()); }

private:
    const char *end() const { return data_ + sizeof(data_); }

    char data_[SIZE]; // 注意不能是指针，否则sizeof(data_)就不是数组的长度
    char *cur_; // 记录当前data_中已经写到了什么位置
    /*------------------------------------*/
    /*|xxxxxxxxxxxxx|<---   avail() ---->|*/
    /*------------------------------------*/
    /*data_        cur_              end()*/
};

} // namespace muduo

#endif /* __MUDUO_FIXED_BUFFER_H_ */

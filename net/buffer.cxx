#include "buffer.h"

#include <errno.h>
#include <sys/uio.h>

namespace muduo {
namespace net {

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::ReadFd(int fd, int *saved_errno) {

    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *saved_errno = errno;
    } else if ((size_t)n <= writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        Append(extrabuf, n - writable);
    }

    return n;
}

void Buffer::Append(const char *data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, BeginWrite());
    HasWritten(len);
}

void Buffer::EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
        MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::MakeSpace(size_t len) {
    if (WritableBytes() >= len) {
        return;
    }

    // len 必须大于可写字节数。
    // if (reader_index_ <= kCheapPrepend) {
    //     if (WritableBytes() < len) {
    //         // FIXME: move readable data
    //         buffer_.resize(writer_index_ + len);
    //     }
    // } else
    // 相当于合并了以上代码
    if (WritableBytes() + PrependableBytes() < len + kCheapPrepend) {
        // FIXME: move readable data
        buffer_.resize(writer_index_ + len);
    } else {
        // move readable data to the front, make space inside buffer
        // 发生了读取数据索引移位才需要移动数据
        assert(kCheapPrepend < reader_index_);
        size_t readable = ReadableBytes();
        std::copy(begin() + reader_index_, begin() + writer_index_,
                  begin() + kCheapPrepend);
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
        assert(readable == ReadableBytes());
    }
}

} // namespace net
} // namespace muduo
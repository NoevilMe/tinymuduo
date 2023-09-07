#include "buffer.h"
#include "inet_socket.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <iostream>

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
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
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

ssize_t Buffer::ReadFd(int fd, int *saved_errno, struct sockaddr_in6 *peer) {
    // https://pubs.opengroup.org/onlinepubs/009696699/functions/recvfrom.html
    // recvfrom. If a message is too long to fit in the supplied buffer, and
    // MSG_PEEK is not set in the flags argument, the excess bytes shall be
    // discarded. refer to
    // https://groups.google.com/g/comp.os.vxworks/c/oJbiziX_Q7U
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
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;

    struct msghdr msg;
    msg.msg_name = peer;
    if (peer) {
        ::bzero(peer, sizeof(struct sockaddr_in6));
        msg.msg_namelen = sizeof(struct sockaddr_in6);
    } else {
        msg.msg_namelen = 0;
    }
    msg.msg_iov = vec;
    msg.msg_iovlen = iovcnt;
    msg.msg_control = nullptr;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    const ssize_t n = ::recvmsg(fd, &msg, 0);
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
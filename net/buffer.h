#ifndef __MUDUO_NET_BUFFER_H_
#define __MUDUO_NET_BUFFER_H_

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <assert.h>

namespace muduo {
namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initial_size = kInitialSize)
        : buffer_(kCheapPrepend + initial_size),
          reader_index_(kCheapPrepend),
          writer_index_(kCheapPrepend) {}

    explicit Buffer(size_t prepend_size, size_t initial_size = kInitialSize)
        : buffer_(prepend_size + initial_size),
          reader_index_(prepend_size),
          writer_index_(prepend_size) {
        assert(prepend_size >= 10);
    }

    size_t ReadableBytes() const { return writer_index_ - reader_index_; }

    size_t WritableBytes() const { return buffer_.size() - writer_index_; }

    size_t PrependableBytes() const { return reader_index_; }

    void EnsureWritableBytes(size_t len);
    void MakeSpace(size_t len);

    void Unwrite(size_t len) {
        assert(len <= ReadableBytes());
        writer_index_ -= len;
    }

    const char *Peek() const { return begin() + reader_index_; }

    const char *FindCRLF() const {
        // FIXME: replace with memmem()?
        const char *crlf = std::search(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
        return crlf == BeginWrite() ? nullptr : crlf;
    }

    const char *FindCRLF(const char *start) const {
        assert(Peek() <= start);
        assert(start <= BeginWrite());
        // FIXME: replace with memmem()?
        const char *crlf = std::search(start, BeginWrite(), kCRLF, kCRLF + 2);
        return crlf == BeginWrite() ? nullptr : crlf;
    }

    ssize_t ReadFd(int fd, int *saved_errno);

    ssize_t ReadFd(int fd, int *saved_errno, struct sockaddr_in6 *peer);

    void Append(const char *data, size_t len);

    std::string RetrieveAllAsString() {
        return RetrieveAsString(ReadableBytes());
    }

    std::string RetrieveAsString(size_t len) {
        assert(len <= ReadableBytes());
        std::string result(Peek(), len);
        Retrieve(len);
        return result;
    }

    // for debug, do not change index
    std::string TryRetrieveAllAsString() {
        return TryRetrieveAsString(ReadableBytes());
    }

    // for debug, do not change index
    std::string TryRetrieveAsString(size_t len) {
        assert(len <= ReadableBytes());
        std::string result(Peek(), len);
        return result;
    }

    void RetrieveUntil(const char *end) {
        assert(Peek() <= end);
        assert(end <= BeginWrite());
        Retrieve(end - Peek());
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void Retrieve(size_t len) {
        assert(len <= ReadableBytes());
        if (len < ReadableBytes()) {
            reader_index_ += len;
        } else {
            RetrieveAll();
        }
    }

    void RetrieveAll() {
        reader_index_ = kCheapPrepend;
        writer_index_ = kCheapPrepend;
    }

    /// @brief 接收以\r\n结尾的第一行数据，
    /// @param include_crlf 返回值是否包含行尾标志\r\n
    /// @param out_line 返回字符串
    /// @return 是否成功
    bool RetrieveCRLFLine(bool include_crlf, std::string &out_line) {
        const char *first_crlf = FindCRLF();
        if (!first_crlf)
            return false;

        out_line.assign(Peek(), include_crlf ? first_crlf + 2 : first_crlf);
        RetrieveUntil(first_crlf + 2);
        return true;
    }

private:
    char *begin() { return &*buffer_.begin(); }

    const char *begin() const { return &*buffer_.begin(); }

    char *BeginWrite() { return begin() + writer_index_; }

    const char *BeginWrite() const { return begin() + writer_index_; }

    void HasWritten(size_t len) {
        assert(len <= WritableBytes());
        writer_index_ += len;
    }

private:
    std::vector<char> buffer_;
    std::size_t reader_index_;
    std::size_t writer_index_;

    static const char kCRLF[];
};
} // namespace net
} // namespace muduo

#endif /* __MUDUO_NET_BUFFER_H_ */

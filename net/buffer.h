#ifndef CDF63D07_06D4_45B3_8D3E_26B7DA494D51
#define CDF63D07_06D4_45B3_8D3E_26B7DA494D51

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

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

    size_t ReadableBytes() const { return writer_index_ - reader_index_; }

    size_t WritableBytes() const { return buffer_.size() - writer_index_; }

    size_t PrependableBytes() const { return reader_index_; }

    void EnsureWritableBytes(size_t len);
    void MakeSpace(size_t len);

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

#endif /* CDF63D07_06D4_45B3_8D3E_26B7DA494D51 */

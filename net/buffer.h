#ifndef CDF63D07_06D4_45B3_8D3E_26B7DA494D51
#define CDF63D07_06D4_45B3_8D3E_26B7DA494D51

#include <algorithm>
#include <cstring>
#include <vector>

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

    const char *peek() const { return begin() + reader_index_; }

private:
    char *begin() { return &*buffer_.begin(); }

    const char *begin() const { return &*buffer_.begin(); }

private:
    std::vector<char> buffer_;
    std::size_t reader_index_;
    std::size_t writer_index_;
};
} // namespace net
} // namespace muduo

#endif /* CDF63D07_06D4_45B3_8D3E_26B7DA494D51 */

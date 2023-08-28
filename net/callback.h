#ifndef F79AA6B7_094F_4CB4_B5CC_3710B459681B
#define F79AA6B7_094F_4CB4_B5CC_3710B459681B

#include "eventloop/timestamp.h"

#include <functional>
#include <memory>

namespace muduo {
namespace net {

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr &, size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *,
                                           event_loop::Timestamp)>;

} // namespace net
} // namespace muduo
#endif /* F79AA6B7_094F_4CB4_B5CC_3710B459681B */

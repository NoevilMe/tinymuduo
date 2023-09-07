#ifndef F79AA6B7_094F_4CB4_B5CC_3710B459681B
#define F79AA6B7_094F_4CB4_B5CC_3710B459681B

#include "eventloop/timestamp.h"

#include <functional>
#include <memory>

struct sockaddr_in6;

namespace muduo {
namespace net {

class Buffer;
class TcpConnection;
class UdpServer;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using UdpServerPtr = std::shared_ptr<UdpServer>;

using TimerCallback = std::function<void()>;

// tcp
using BeforeReadingCallback = std::function<void(const TcpConnectionPtr &)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr &, size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *,
                                           event_loop::Timestamp)>;

// udp
using UdpServerBindedCallback = std::function<void(const UdpServerPtr &)>;
using UdpServerCloseCallback = std::function<void(const UdpServerPtr &)>;
using UdpServerMessageCallback =
    std::function<void(const UdpServerPtr &, Buffer *, struct sockaddr_in6 *,
                       event_loop::Timestamp)>;

} // namespace net
} // namespace muduo
#endif /* F79AA6B7_094F_4CB4_B5CC_3710B459681B */

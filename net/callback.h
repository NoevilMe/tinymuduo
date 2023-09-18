#ifndef __MUDUO_NET_CALLBACK_H_
#define __MUDUO_NET_CALLBACK_H_

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
#endif /* __MUDUO_NET_CALLBACK_H_ */

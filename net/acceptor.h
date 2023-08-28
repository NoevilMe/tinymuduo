#ifndef __MUDUO_NET_ACCEPTOR_H_
#define __MUDUO_NET_ACCEPTOR_H_

#include "eventloop/noncopyable.h"
#include "eventloop/timestamp.h"
#include "inet_socket.h"

#include <functional>
#include <memory>

namespace muduo {

namespace event_loop {
class Channel;
class EventLoop;
class Timestamp;
} // namespace event_loop

namespace net {

class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : event_loop::Noncopyable {
public:
    using NewConnectionCallback =
        std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(event_loop::EventLoop *loop, const InetAddress &listen_addr,
             bool reuseport);
    ~Acceptor();

    void set_new_connection_callback(const NewConnectionCallback &cb) {
        new_connection_callback_ = cb;
    }

    void Listen();

    bool listening() const { return listening_; }

private:
    void HandleRead(event_loop::Timestamp);

    event_loop::EventLoop *loop_;
    InetAddress serv_address_;
    Socket accept_socket_;
    std::unique_ptr<event_loop::Channel> accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listening_;
    int idle_fd_;
};

} // namespace net
} // namespace muduo

#endif /* __MUDUO_NET_ACCEPTOR_H_ */

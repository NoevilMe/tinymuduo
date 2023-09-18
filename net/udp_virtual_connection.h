#ifndef __MUDUO_NET_UDP_VIRTUAL_CONNECTION_H__
#define __MUDUO_NET_UDP_VIRTUAL_CONNECTION_H__

#include "buffer.h"
#include "callback.h"
#include "eventloop/eventloop.h"
#include "inet_address.h"
#include "inet_socket.h"
#include "udp_server.h"

#include <memory>

namespace muduo {
namespace net {

class UdpVirtualConnection
    : Noncopyable,
      public std::enable_shared_from_this<UdpVirtualConnection> {
public:
    UdpVirtualConnection(event_loop::EventLoop *loop, const std::string &name,
                         int sockfd, const InetAddress &local_addr,
                         const InetAddress &peer_addr,
                         bool match_peer_ip = true);
    ~UdpVirtualConnection();

    event_loop::EventLoop *loop() const {
        return udp_server_ ? udp_server_->loop() : nullptr;
    }
    const std::string &name() const { return udp_server_->name(); }
    const InetAddress &local_addr() const { return udp_server_->local_addr(); }
    const InetAddress &peer_addr() const { return peer_addr_; }

    bool Binded() const { return udp_server_ ? udp_server_->Binded() : false; }

    bool Bind();
    bool Bind(const InetAddress &new_addr);

    void Close();

    void Send(const void *data, int len);
    void Send(const std::string &message);

    void SetSendBufSize(size_t size);

    void SetRecvBufSize(size_t size);

    // bind成功回调
    void set_binded_callback(const UdpServerBindedCallback &cb) {
        if (udp_server_)
            udp_server_->set_binded_callback(cb);
    }

    // 收到消息
    void set_message_callback(const UdpServerMessageCallback &cb) {
        message_callback_ = cb;
    }

    void set_close_callback(const UdpServerCloseCallback &cb) {
        if (udp_server_)
            udp_server_->set_close_callback(cb);
    }

    // should be called only once
    void BindingFinished() {
        if (udp_server_)
            udp_server_->BindingFinished();
    }

private:
    void OnMessage(const UdpServerPtr &server, Buffer *buf,
                   struct sockaddr_in6 *addr, event_loop::Timestamp);

private:
    std::shared_ptr<UdpServer> udp_server_;

    int sockfd_;
    InetAddress peer_addr_;
    bool match_peer_ip_;

    UdpServerMessageCallback message_callback_;
};

} // namespace net
} // namespace muduo

#endif // __MUDUO_NET_UDP_VIRTUAL_CONNECTION_H__
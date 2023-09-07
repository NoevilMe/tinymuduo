#include "udp_virtual_connection.h"
#include "eventloop/channel.h"
#include "logger/logger.h"

namespace muduo {
namespace net {

UdpVirtualConnection::UdpVirtualConnection(event_loop::EventLoop *loop,
                                           const std::string &name, int sockfd,
                                           const InetAddress &local_addr,
                                           const InetAddress &peer_addr,
                                           bool match_peer_ip)
    : udp_server_(new UdpServer(loop, name, sockfd, local_addr)),
      sockfd_(sockfd),
      peer_addr_(peer_addr),
      match_peer_ip_(match_peer_ip) {

    udp_server_->set_message_callback(std::bind(
        &UdpVirtualConnection::OnMessage, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    LOG_DEBUG << "UdpVirtualConnection::ctor[" << name << "] at " << this
              << " fd=" << sockfd;
}

UdpVirtualConnection::~UdpVirtualConnection() {}

bool UdpVirtualConnection::Bind() { return udp_server_->Bind(); }

bool UdpVirtualConnection::Bind(const InetAddress &new_addr) {
    return udp_server_->Bind(new_addr);
}

void UdpVirtualConnection::Close() {
    if (udp_server_)
        udp_server_->Close();
}

void UdpVirtualConnection::Send(const void *data, int len) {
    if (udp_server_)
        udp_server_->Send(data, len, &peer_addr_);
}

void UdpVirtualConnection::Send(const std::string &message) {
    if (udp_server_)
        udp_server_->Send(message, &peer_addr_);
}

void UdpVirtualConnection::OnMessage(const UdpServerPtr &server, Buffer *buf,
                                     struct sockaddr_in6 *addr,
                                     event_loop::Timestamp timestamp) {

    if (message_callback_) {
        if (!match_peer_ip_) {
            message_callback_(server, buf, addr, timestamp);
        } else if (peer_addr_.MatchIp(*addr)) {
            message_callback_(server, buf, addr, timestamp);
        } else {
            LOG_ERROR << "UdpVirtualConnection::OnMessage, peer address "
                      << sockets::ToIp((const struct sockaddr *)addr)
                      << " doesn't match " << peer_addr_.Ip();
        }
    }
}

} // namespace net
} // namespace muduo
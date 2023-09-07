#include "eventloop/eventloop.h"
#include "logger/logger.h"
#include "net/acceptor.h"
#include "net/callback.h"
#include "net/inet_address.h"
#include "net/inet_socket.h"
#include "net/udp_server.h"
#include "net/udp_virtual_connection.h"

#include <iostream>
#include <unistd.h>

void on_server_message(const muduo::net::UdpServerPtr &server,
                       muduo::net::Buffer *buf, struct sockaddr_in6 *addr,
                       muduo::event_loop::Timestamp,
                       const muduo::net::InetAddress *peer) {

    LOG_INFO << muduo::net::sockets::ToIpPort((const sockaddr *)addr)
             << " receive msg: " << buf->RetrieveAllAsString();
    server->Send("response", peer);
}

int main() {
    muduo::log::Logger::set_log_level(muduo::log::Logger::TRACE);
    muduo::event_loop::EventLoop loop;

    int sockfd = muduo::net::sockets::CreateNonblockingUdp(AF_INET);

    muduo::net::InetAddress addr(38888);
    muduo::net::InetAddress peer("10.10.10.178", 29000);

    int socked = muduo::net::sockets::CreateNonblockingUdp(AF_INET);

    std::shared_ptr<muduo::net::UdpVirtualConnection> server(
        new muduo::net::UdpVirtualConnection(&loop, "udp conn", socked, addr,
                                             peer));

    server->set_message_callback(std::bind(
        on_server_message, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4, &peer));
    server->set_close_callback([](const muduo::net::UdpServerPtr &server) {
        LOG_INFO << "server closed";
    });

    if (server->Bind()) {
        LOG_INFO << " bind success";
    } else {
        LOG_ERROR << "bind failure";
        return 1;
    }

    server->BindingFinished();

    loop.RunAfter(60, [=]() { server->Close(); });

    // 如果我们在用户空间 close 了 file descriptor ，就无法再通过 epoll_ctl
    // 去控制它了； 不会触发任何回调
    loop.RunAfter(20, [=]() {
        LOG_INFO << "close fd " << sockfd;
        close(sockfd);
    });

    loop.RunAfter(10, [=]() { server->Send("hello udp server 1"); });

    loop.RunAfter(30, [=]() { server->Send("hello udp server 2"); });

    loop.Loop();

    return 0;
}
#include "eventloop/eventloop.h"
#include "logger/logger.h"
#include "net/acceptor.h"
#include "net/callback.h"
#include "net/inet_address.h"
#include "net/inet_socket.h"
// #include "net/udp_virtual_connection.h"
#include "net/udp_server.h"

#include <iostream>
#include <unistd.h>

// void(const UdpVirtualConnectionPtr &,
//   Buffer *, event_loop::Timestamp)
// void on_message(const muduo::net::UdpVirtualConnectionPtr &conn,
//                 muduo::net::Buffer *buf, muduo::event_loop::Timestamp) {

//     LOG_INFO << conn->name() << " receive msg: " <<
//     buf->RetrieveAllAsString(); conn->Send("hello udp");
// }

void on_server_message(const muduo::net::UdpServerPtr &server,
                       muduo::net::Buffer *buf, struct sockaddr_in6 *addr,
                       muduo::event_loop::Timestamp) {

    LOG_INFO << muduo::net::sockets::ToIpPort((const sockaddr *)addr)
             << " receive msg: " << buf->RetrieveAllAsString();
}

int main() {
    muduo::log::Logger::set_log_level(muduo::log::Logger::TRACE);
    muduo::event_loop::EventLoop loop;

    muduo::net::InetAddress addr(38888);

#if 1
    int sockfd = muduo::net::sockets::CreateNonblockingUdp(AF_INET);

    std::shared_ptr<muduo::net::UdpServer> server(
        new muduo::net::UdpServer(&loop, "udp server", sockfd, addr));

    server->set_message_callback(std::bind(
        on_server_message, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    server->set_close_callback([](const muduo::net::UdpServerPtr &server) {
        LOG_INFO << "server closed";
    });
#else
    muduo::net::InetAddress peer("10.10.10.178", 38880);

    int socked = muduo::net::sockets::CreateNonblockingUdp(AF_INET);

    std::shared_ptr<muduo::net::UdpVirtualConnection> server(
        new muduo::net::UdpVirtualConnection(&loop, "udp server", socked, addr,
                                             peer));

    server->set_message_callback(std::bind(on_message, std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3));
#endif

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

    loop.RunAfter(10, [=]() {
        muduo::net::InetAddress peer("10.10.10.178", 29000);
        server->Send("hello udp server 1", &peer);
    });

    loop.RunAfter(30, [=]() {
        muduo::net::InetAddress peer("10.10.10.178", 29000);
        server->Send("hello udp server 2", &peer);
    });

    loop.Loop();

    return 0;
}
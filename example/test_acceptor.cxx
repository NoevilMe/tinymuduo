#include "eventloop/eventloop.h"
#include "logger/logger.h"
#include "net/acceptor.h"
#include "net/inet_address.h"

#include <iostream>
#include <unistd.h>

void new_conn(int sockfd, const muduo::net::InetAddress &addr) {
    LOG_INFO << "new connection from " << addr.Ip() << " port " << addr.Port()
             << " -- " << addr.IpPort() << ", fd " << sockfd;
    close(sockfd);
}

int main() {
    muduo::log::Logger::set_log_level(muduo::log::Logger::DEBUG);
    muduo::event_loop::EventLoop loop;

    muduo::net::InetAddress addr(38880);

    muduo::net::Acceptor acpt(&loop, addr, true);
    acpt.set_new_connection_callback(
        std::bind(new_conn, std::placeholders::_1, std::placeholders::_2));

    acpt.Listen();

    loop.Loop();

    return 0;
}
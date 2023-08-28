#include "eventloop/eventloop.h"
#include "logger/logger.h"
#include "net/acceptor.h"
#include "net/inet_address.h"
#include "net/tcp_connection.h"
#include "net/tcp_server.h"

#include <iostream>
#include <unistd.h>

void new_conn(const muduo::net::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        LOG_INFO << "new connection from " << conn->peer_addr().IpPort();
        // close(sockfd);
        conn->Shutdown();
    } else {
        LOG_INFO << "destory connection from " << conn->peer_addr().IpPort();
    }
}

int main() {
    muduo::log::Logger::set_log_level(muduo::log::Logger::DEBUG);
    muduo::event_loop::EventLoop loop;

    muduo::net::InetAddress addr(38880);

    muduo::net::TcpServer acpt(&loop, addr, "Sample Tcp Server");
    acpt.set_connection_callback(std::bind(new_conn, std::placeholders::_1));

    acpt.Start();

    loop.Loop();

    return 0;
}
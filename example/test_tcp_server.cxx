#include "eventloop/eventloop.h"
#include "logger/logger.h"
#include "net/acceptor.h"
#include "net/callback.h"
#include "net/inet_address.h"
#include "net/tcp_connection.h"
#include "net/tcp_server.h"

#include <iostream>
#include <unistd.h>

void new_conn(const muduo::net::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        LOG_INFO << "new connection from " << conn->peer_addr().IpPort();
        // close(sockfd);
        // conn->Shutdown();
    } else {
        LOG_INFO << "destory connection from " << conn->peer_addr().IpPort();
    }
}

void on_message(const muduo::net::TcpConnectionPtr &conn,
                muduo::net::Buffer *buf, muduo::event_loop::Timestamp) {

    LOG_INFO << conn->name() << " receive msg: " << buf->RetrieveAllAsString();
    conn->Send("55555");
}

int main() {
    muduo::log::Logger::set_log_level(muduo::log::Logger::TRACE);
    muduo::event_loop::EventLoop loop;

    muduo::net::InetAddress addr(38880);

    muduo::net::TcpServer server(&loop, addr, "Sample Tcp Server");
    server.set_connection_callback(std::bind(new_conn, std::placeholders::_1));
    server.set_message_callback(std::bind(on_message, std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3));

    server.SetThreadNum(5);

    server.Start();

    loop.Loop();

    return 0;
}
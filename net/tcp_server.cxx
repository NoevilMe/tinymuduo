#include "tcp_server.h"
#include "logger/logger.h"
#include "tcp_connection.h"

namespace muduo {
namespace net {
TcpServer::TcpServer(event_loop::EventLoop *loop,
                     const InetAddress &listen_addr, const std::string &name,
                     bool reuse_port)
    : loop_(loop),
      name_(name),
      listen_ip_port_(listen_addr.IpPort()),
      started_(false),
      thread_pool_(new event_loop::EventLoopThreadPool(loop, name_)),
      acceptor_(new Acceptor(loop, listen_addr, reuse_port)),
      next_conn_id_(1) {
    acceptor_->set_new_connection_callback(
        std::bind(&TcpServer::NewConnection, this, std::placeholders::_1,
                  std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();

    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->loop()->RunInLoop(
            std::bind(&TcpConnection::ConnectDestroyed, conn));
    }
}

void TcpServer::SetThreadNum(int threads) {
    assert(0 <= threads);
    thread_pool_->SetThreadNum(threads);
}

void TcpServer::Start() {
    if (started_.exchange(true) == false) {
        thread_pool_->Start(thread_init_callback_);

        assert(!acceptor_->listening());
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
    }
}

void TcpServer::NewConnection(int sockfd, const InetAddress &peer_addr) {
    loop_->AssertInLoopThread();

    auto ioloop = thread_pool_->GetNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", listen_ip_port_.data(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;

    LOG_INFO << "TcpServer::NewConnection [" << name_ << "] - new connection["
             << conn_name << "] from " << peer_addr.IpPort();

    InetAddress local_addr(sockets::GetLocalAddr(sockfd));
    // // FIXME poll with zero timeout to double confirm the new connection
    // // FIXME use make_shared if necessary
    TcpConnectionPtr conn(
        new TcpConnection(ioloop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(std::bind(&TcpServer::RemoveConnection, this,
                                       std::placeholders::_1)); // FIXME: unsafe

    ioloop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "TcpServer::RemoveConnection [" << name_ << "] - connection "
             << conn->name();
    // FIXME: unsafe
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->AssertInLoopThread();
    LOG_INFO << "TcpServer::RemoveConnectionInLoop [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    auto *ioLoop = conn->loop();
    ioLoop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}
} // namespace net
} // namespace muduo
#ifndef __MUDUO_NET_TCP_CONNECTION_H_
#define __MUDUO_NET_TCP_CONNECTION_H_

#include "buffer.h"
#include "callback.h"
#include "eventloop/eventloop.h"
#include "inet_address.h"
#include "inet_socket.h"

#include <memory>

namespace muduo {
namespace net {

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(event_loop::EventLoop *loop, const std::string &name,
                  int sockfd, const InetAddress &local_addr,
                  const InetAddress &peer_addr);
    ~TcpConnection();

    event_loop::EventLoop *loop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &local_addr() const { return local_addr_; }
    const InetAddress &peer_addr() const { return peer_addr_; }

    void Send(const void *data, int len);
    void Send(const std::string &message);

    void Shutdown(); // NOT thread safe, no simultaneous calling

    bool Connected() const { return state_ == kConnected; }
    bool Disconnected() const { return state_ == kDisconnected; }

    void SetTcpNoDelay(bool on);

    // 连接已经建立，但是还没开始读取数据（可以用来设置message callback等）
    void set_before_reading_callback(const BeforeReadingCallback &cb) {
        before_reading_callback_ = cb;
    }

    // 连接建立（已经开始读取数据）和断开的时候回调
    void set_connection_callback(const ConnectionCallback &cb) {
        connection_callback_ = cb;
    }

    // 收到消息
    void set_message_callback(const MessageCallback &cb) {
        message_callback_ = cb;
    }

    // 发送数据完毕
    void set_write_complete_callback(const WriteCompleteCallback &cb) {
        write_complete_callback_ = cb;
    }

    /// Internal use only.
    void set_close_callback(const CloseCallback &cb) { close_callback_ = cb; }

    // called when TcpServer accepts a new connection
    void ConnectEstablished(); // should be called only once
    // called when TcpServer has removed me from its map
    void ConnectDestroyed(); // should be called only once

private:
    enum ConnectionState {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void SetState(ConnectionState s) { state_ = s; }

    void ShutdownInLoop();
    void SendInLoop(const std::string &message);
    void SendInLoop(const void *message, size_t len);

    void HandleRead(event_loop::Timestamp poll_time);
    void HandleWrite();
    void HandleClose();
    void HandleError();

private:
    event_loop::EventLoop *loop_;
    const std::string name_;
    const InetAddress local_addr_;
    const InetAddress peer_addr_;

    ConnectionState state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<event_loop::Channel> channel_;

    BeforeReadingCallback before_reading_callback_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;

    Buffer receive_buffer_;
    Buffer send_buffer_; // FIXME: use list<Buffer> as output buffer.
};

} // namespace net
} // namespace  muduo

#endif /* __MUDUO_NET_TCP_CONNECTION_H_ */

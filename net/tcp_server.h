#ifndef __MUDUO_NET_TCP_SERVER_H_
#define __MUDUO_NET_TCP_SERVER_H_

#include "acceptor.h"
#include "callback.h"
#include "eventloop/event_loop.h"
#include "eventloop/event_loop_threadpool.h"
#include "eventloop/noncopyable.h"
#include "inet_address.h"

namespace muduo {
namespace net {

class TcpServer : Noncopyable {

public:
    using ThreadInitCallback = std::function<void(event_loop::EventLoop *)>;

    TcpServer(event_loop::EventLoop *loop, const InetAddress &listen_addr,
              const std::string &name, bool reuse_port = false);
    ~TcpServer();

    const std::string &listen_ip_port() const { return listen_ip_port_; }
    const std::string &name() const { return name_; }
    event_loop::EventLoop *loop() const { return loop_; }

    /// Set the number of threads for handling input.
    ///
    /// Always accepts new connection in loop's thread.
    /// Must be called before @c start
    /// @param threads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void SetThreadNum(int threads);

    /// Starts the server if it's not listening.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void Start();

    /// Not thread safe.
    /// 连接建立, 读取数据之前调用
    void set_before_reading_callback(const BeforeReadingCallback &cb) {
        before_reading_callback_ = cb;
    }

    /// Not thread safe.
    /// 连接建立和断开时候调用
    void set_connection_callback(const ConnectionCallback &cb) {
        connection_callback_ = cb;
    }
    /// Not thread safe.
    /// 连接消息到达
    void set_message_callback(const MessageCallback &cb) {
        message_callback_ = cb;
    }
    /// Not thread safe.
    /// 连接数据发送完毕
    void set_write_complete_callback(const WriteCompleteCallback &cb) {
        write_complete_callback_ = cb;
    }

    void set_thread_init_callback(const ThreadInitCallback &cb) {
        thread_init_callback_ = cb;
    }

private:
    /// Not thread safe, but in loop
    void NewConnection(int sockfd, const InetAddress &peer_addr);
    /// Thread safe.
    void RemoveConnection(const TcpConnectionPtr &conn);
    /// Not thread safe, but in loop
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    // 用于接受新连接
    event_loop::EventLoop *loop_;
    const std::string name_;

    std::string listen_ip_port_;

    BeforeReadingCallback before_reading_callback_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ThreadInitCallback thread_init_callback_;

    std::atomic_bool started_;
    std::shared_ptr<event_loop::EventLoopThreadPool> thread_pool_;
    std::unique_ptr<Acceptor> acceptor_;
    // always in loop thread
    int next_conn_id_;
    // client connections
    std::map<std::string, TcpConnectionPtr> connections_;
};

} // namespace net
} // namespace muduo
#endif /* __MUDUO_NET_TCP_SERVER_H_ */

#ifndef AC995845_A409_4FD6_BC29_FB96968A8D20
#define AC995845_A409_4FD6_BC29_FB96968A8D20

#include "acceptor.h"
#include "callback.h"
#include "eventloop/event_loop.h"
#include "eventloop/noncopyable.h"
#include "inet_address.h"

namespace muduo {
namespace net {

class TcpServer : event_loop::Noncopyable {

public:
    TcpServer(event_loop::EventLoop *loop, const InetAddress &listen_addr,
              const std::string &name, bool reuse_port = false);
    ~TcpServer();

    const std::string &listen_ip_port() const { return listen_ip_port_; }
    const std::string &name() const { return name_; }
    event_loop::EventLoop *loop() const { return loop_; }

    /// Starts the server if it's not listening.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void Start();

    /// Not thread safe.
    void set_connection_callback(const ConnectionCallback &cb) {
        connection_callback_ = cb;
    }
    /// Not thread safe.
    void set_message_callback(const MessageCallback &cb) {
        message_callback_ = cb;
    }
    /// Not thread safe.
    void set_write_complete_callback(const WriteCompleteCallback &cb) {
        write_complete_callback_ = cb;
    }

private:
    /// Not thread safe, but in loop
    void NewConnection(int sockfd, const InetAddress &peer_addr);
    /// Thread safe.
    void RemoveConnection(const TcpConnectionPtr &conn);
    /// Not thread safe, but in loop
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    event_loop::EventLoop *loop_;
    const std::string name_;

    std::string listen_ip_port_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;

    std::unique_ptr<Acceptor> acceptor_;
    // always in loop thread
    int next_conn_id_;
    // client connections
    std::map<std::string, TcpConnectionPtr> connections_;
};

} // namespace net
} // namespace muduo
#endif /* AC995845_A409_4FD6_BC29_FB96968A8D20 */

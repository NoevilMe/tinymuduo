#ifndef DBAA5A75_D448_4471_8FF8_443B94BD26B7
#define DBAA5A75_D448_4471_8FF8_443B94BD26B7

#include "buffer.h"
#include "callback.h"
#include "eventloop/eventloop.h"
#include "inet_address.h"
#include "inet_socket.h"

#include <memory>

namespace muduo {
namespace net {

class UdpServer : event_loop::Noncopyable,
                  public std::enable_shared_from_this<UdpServer> {
public:
    UdpServer(event_loop::EventLoop *loop, const std::string &name, int sockfd,
              const InetAddress &local_addr);
    ~UdpServer();

    event_loop::EventLoop *loop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &local_addr() const { return local_addr_; }

    bool Binded() const { return state_ == kBinded; }

    bool Bind();
    bool Bind(const InetAddress &new_addr);

    void Close();

    void Send(const void *data, int len, const InetAddress *peer);
    void Send(const std::string &message, const InetAddress *peer);
    void SendInLoop(const std::string &message, const InetAddress &peer);
    void SendInLoop(const void *data, size_t len, const InetAddress *peer);

    void SetSendBufSize(size_t size);

    void SetRecvBufSize(size_t size);

    // bind成功回调
    void set_binded_callback(const UdpServerBindedCallback &cb) {
        binded_callback_ = cb;
    }

    // 收到消息
    void set_message_callback(const UdpServerMessageCallback &cb) {
        message_callback_ = cb;
    }

    void set_close_callback(const UdpServerCloseCallback &cb) {
        close_callback_ = cb;
    }

    void BindingFinished(); // should be called only once

private:
    enum BindState { kBinded, kUnbinded };

    void SetState(BindState s) { state_ = s; }

    void HandleRead(event_loop::Timestamp poll_time);
    void HandleClose();
    void HandleError();

private:
    event_loop::EventLoop *loop_;
    const std::string name_;
    InetAddress local_addr_;

    BindState state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<event_loop::Channel> channel_;

    UdpServerBindedCallback binded_callback_;
    UdpServerMessageCallback message_callback_;
    UdpServerCloseCallback close_callback_;

    Buffer receive_buffer_;
};

} // namespace net
} // namespace muduo

#endif // __UDP_VIRTUAL_CONNECTION_H__

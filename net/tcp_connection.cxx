#include "tcp_connection.h"
#include "eventloop/channel.h"
#include "logger/logger.h"

namespace muduo {
namespace net {
TcpConnection::TcpConnection(event_loop::EventLoop *loop,
                             const std::string &name, int sockfd,
                             const InetAddress &local_addr,
                             const InetAddress &peer_addr)
    : loop_(loop),
      name_(name),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new event_loop::Channel(loop, sockfd)) {
    channel_->set_read_callback(
        std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
    channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->set_close_callback(std::bind(&TcpConnection::HandleClose, this));
    channel_->set_error_callback(std::bind(&TcpConnection::HandleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {}

void TcpConnection::Send(const void *message, int len) {
    if (state_ == kConnected) {
        if (loop_->IsInLoopThread()) {
            SendInLoop(message, len);
        } else {
            // 其他线程复制数据
            void (TcpConnection::*fp)(const std::string &message) =
                &TcpConnection::SendInLoop;
            loop_->RunInLoop(
                std::bind(fp, this, std::string((char *)message, len)));
        }
    }
}

void TcpConnection::Send(const std::string &message) {
    Send(message.data(), message.length());
}

void TcpConnection::Shutdown() {
    // FIXME: use compare and swap
    if (state_ == kConnected) {
        SetState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::SetTcpNoDelay(bool on) { socket_->SetTcpNoDelay(on); }

void TcpConnection::ShutdownInLoop() {
    LOG_DEBUG << "TcpConnection::ShutdownInLoop " << channel_->fd();
    loop_->AssertInLoopThread();
    if (!channel_->IsWriting()) {
        // we are not writing
        socket_->ShutdownWrite();
    }
}

void TcpConnection::SendInLoop(const std::string &message) {
    SendInLoop(message.data(), message.length());
}

void TcpConnection::SendInLoop(const void *data, size_t len) {
    loop_->AssertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->IsWriting() && send_buffer_.ReadableBytes() == 0) {
        nwrote = sockets::Write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_) {
                loop_->QueueInLoop(
                    std::bind(write_complete_callback_, shared_from_this()));
            }
        } else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::SendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = send_buffer_.ReadableBytes();
        // TODO:
        // if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_
        // &&
        //     highWaterMarkCallback_) {
        //     loop_->queueInLoop(std::bind(highWaterMarkCallback_,
        //                                  shared_from_this(),
        //                                  oldLen + remaining));
        // }
        send_buffer_.Append(static_cast<const char *>(data) + nwrote,
                            remaining);
        if (!channel_->IsWriting()) {
            channel_->EnableWriting();
        }
    }
}

void TcpConnection::HandleRead(event_loop::Timestamp poll_time) {
    loop_->AssertInLoopThread();
    int saved_errno = 0;
    ssize_t n = receive_buffer_.ReadFd(channel_->fd(), &saved_errno);
    LOG_TRACE << "TcpConnection::HandleRead length " << n;
    if (n > 0) {
        if (message_callback_)
            message_callback_(shared_from_this(), &receive_buffer_, poll_time);
    } else if (n == 0) {
        HandleClose();
    } else {
        errno = saved_errno;
        LOG_SYSERR << "TcpConnection::handleRead";
        HandleError();
    }
}

void TcpConnection::HandleWrite() {
    LOG_TRACE << "TcpConnection::HandleWrite " << channel_->fd();

    loop_->AssertInLoopThread();
    if (channel_->IsWriting()) {
        ssize_t n = sockets::Write(channel_->fd(), send_buffer_.Peek(),
                                   send_buffer_.ReadableBytes());
        if (n > 0) {
            send_buffer_.Retrieve(n); // 改变可读数据索引
            if (send_buffer_.ReadableBytes() == 0) {
                // 没有数据就停止监控可写事件，避免不停回调
                channel_->DisableWriting();
                if (write_complete_callback_) {
                    loop_->QueueInLoop(std::bind(write_complete_callback_,
                                                 shared_from_this()));
                }
                // 正在主动关闭连接
                if (state_ == kDisconnecting) {
                    ShutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TcpConnection::HandleWrite";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    } else {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                  << " is down, no more writing";
    }
}

void TcpConnection::HandleClose() {
    LOG_DEBUG << "TcpConnection::HandleClose " << channel_->fd();

    loop_->AssertInLoopThread();
    // LOG_TRACE << "fd = " << channel_->fd() << " state = " <<
    // stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    SetState(kDisconnected);
    channel_->DisableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connection_callback_(guardThis);
    // must be the last line
    close_callback_(guardThis);
}

void TcpConnection::HandleError() {
    int err = sockets::GetSocketErrno(channel_->fd());
    LOG_ERROR << "TcpConnection::HandleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::ConnectEstablished() {
    loop_->AssertInLoopThread();
    assert(state_ == kConnecting);
    SetState(kConnected);
    channel_->Tie(shared_from_this());
    channel_->EnableReading();

    connection_callback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed() {
    LOG_DEBUG << "TcpConnection::ConnectDestroyed " << channel_->fd();

    loop_->AssertInLoopThread();
    if (state_ == kConnected) {
        SetState(kDisconnected);
        channel_->DisableAll();

        connection_callback_(shared_from_this());
    }
    channel_->RemoveFromLoop();
}

} // namespace net
} // namespace muduo
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
    // socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {}

void TcpConnection::Shutdown() {
    // FIXME: use compare and swap
    if (state_ == kConnected) {
        SetState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop() {
    LOG_DEBUG << "TcpConnection::ShutdownInLoop " << channel_->fd();
    loop_->AssertInLoopThread();
    if (!channel_->IsWriting()) {
        // we are not writing
        socket_->ShutdownWrite();
    }
}

void TcpConnection::HandleRead(event_loop::Timestamp poll_time) {
    LOG_DEBUG << "TcpConnection::HandleRead " << channel_->fd();

    loop_->AssertInLoopThread();
    int savedErrno = 0;

    char buffer[8192] = {0};
    int ret = ::recv(channel_->fd(), buffer, 8192, 0);

    if (ret == 0) {
        HandleClose();
    } else if (ret < 0) {
        HandleError();
    }

    // ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    // if (n > 0) {
    //     messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    // } else if (n == 0) {
    //     handleClose();
    // } else {
    //     errno = savedErrno;
    //     LOG_SYSERR << "TcpConnection::handleRead";
    //     handleError();
    // }
}

void TcpConnection::HandleWrite() {
    LOG_DEBUG << "TcpConnection::HandleWrite " << channel_->fd();

    //      loop_->assertInLoopThread();
    //   if (channel_->isWriting())
    //   {
    //     ssize_t n = sockets::write(channel_->fd(),
    //                                outputBuffer_.peek(),
    //                                outputBuffer_.readableBytes());
    //     if (n > 0)
    //     {
    //       outputBuffer_.retrieve(n);
    //       if (outputBuffer_.readableBytes() == 0)
    //       {
    //         channel_->disableWriting();
    //         if (writeCompleteCallback_)
    //         {
    //           loop_->queueInLoop(std::bind(writeCompleteCallback_,
    //           shared_from_this()));
    //         }
    //         if (state_ == kDisconnecting)
    //         {
    //           shutdownInLoop();
    //         }
    //       }
    //     }
    //     else
    //     {
    //       LOG_SYSERR << "TcpConnection::handleWrite";
    //       // if (state_ == kDisconnecting)
    //       // {
    //       //   shutdownInLoop();
    //       // }
    //     }
    //   }
    //   else
    //   {
    //     LOG_TRACE << "Connection fd = " << channel_->fd()
    //               << " is down, no more writing";
    //   }
}

void TcpConnection::HandleClose() {
    LOG_DEBUG << "TcpConnection::HandleClose " << channel_->fd();

    loop_->AssertInLoopThread();
    // LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
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
    LOG_DEBUG << "TcpConnection::HandleError " << channel_->fd();
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

        // TODO:
        // connection_callback_(shared_from_this());
    }
    channel_->RemoveFromLoop();
}

} // namespace net
} // namespace muduo
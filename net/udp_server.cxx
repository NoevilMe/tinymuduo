#include "udp_server.h"
#include "eventloop/channel.h"
#include "logger/logger.h"

namespace muduo {
namespace net {

// The maximum IP packet size is 65,535 bytes。IPv4 uses 20 bytes and
// UDP 8 bytes
constexpr unsigned int kUdpPacketMaxSize = 65535 - 20 - 8;

UdpServer::UdpServer(event_loop::EventLoop *loop, const std::string &name,
                     int sockfd, const InetAddress &local_addr)
    : loop_(loop),
      name_(name),
      local_addr_(local_addr),
      state_(kUnbinded),
      socket_(new Socket(sockfd)),
      channel_(new event_loop::Channel(loop, sockfd)) {
    channel_->set_read_callback(
        std::bind(&UdpServer::HandleRead, this, std::placeholders::_1));
    // TODO： UDP无法触发close回调
    channel_->set_close_callback(std::bind(&UdpServer::HandleClose, this));
    channel_->set_error_callback(std::bind(&UdpServer::HandleError, this));
    LOG_DEBUG << "UdpServer::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
}

UdpServer::~UdpServer() {}

bool UdpServer::Bind() {
    assert(kUnbinded == state_);
    bool res = socket_->BindAddressAlive(local_addr_);
    LOG_DEBUG << "bind on " << local_addr_.IpPort() << " " << res;
    return res;
}

bool UdpServer::Bind(const InetAddress &new_addr) {
    assert(kUnbinded == state_);
    if (socket_->BindAddressAlive(new_addr)) {
        local_addr_ = new_addr;
        return true;
    } else {
        return false;
    }
}

void UdpServer::Close() {
    channel_->DisableAll();
    channel_->RemoveFromLoop();
    channel_.reset();

    socket_.reset();

    if (close_callback_)
        close_callback_(shared_from_this());
}

void UdpServer::Send(const void *data, int len, const InetAddress *peer) {
    if (loop_->IsInLoopThread()) {
        SendInLoop(data, len, peer);
    } else {
        // 其他线程复制数据
        void (UdpServer::*fp)(const std::string &message,
                              const InetAddress &peer) = &UdpServer::SendInLoop;
        loop_->RunInLoop(
            std::bind(fp, this, std::string((char *)data, len), *peer));
    }
}

void UdpServer::Send(const std::string &message, const InetAddress *peer) {
    Send(message.data(), message.length(), peer);
}

void UdpServer::SendInLoop(const std::string &message,
                           const InetAddress &peer) {
    SendInLoop(message.data(), message.length(), &peer);
}

void UdpServer::SendInLoop(const void *data, size_t len,
                           const InetAddress *peer) {
    loop_->AssertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;

    // Hard to provide peer address in the write callback. writing directly

    while (remaining > 0) {
        size_t send_len =
            remaining > kUdpPacketMaxSize ? kUdpPacketMaxSize : remaining;
        nwrote = sockets::SendTo(channel_->fd(), data, send_len,
                                 peer->GetSockAddr());

        LOG_TRACE << "SendTo " << peer->IpPort() << " length " << nwrote;
        if (nwrote > 0) {
            // TODO: protocol prossibly does't work!!!
            remaining -= nwrote;
        } else if (nwrote == 0) {
            // FIXME:
            LOG_ERROR << "UdpServer::SendInLoop send data length 0";
            break;
        } else { // nwrote < 0
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "UdpServer::SendInLoop";
            }
            break;
        }
    }
}

void UdpServer::SetSendBufSize(size_t size) { socket_->SetSendBufSize(size); }

void UdpServer::SetRecvBufSize(size_t size) { socket_->SetRecvBufSize(size); }

void UdpServer::BindingFinished() {
    loop_->AssertInLoopThread();
    assert(state_ == kUnbinded);
    SetState(kBinded);

    channel_->Tie(shared_from_this());
    channel_->EnableReading();

    if (binded_callback_)
        binded_callback_(shared_from_this());
}

void UdpServer::HandleRead(event_loop::Timestamp poll_time) {
    loop_->AssertInLoopThread();
    int saved_errno = 0;

    struct sockaddr_in6 peer;
    ssize_t n = receive_buffer_.ReadFd(channel_->fd(), &saved_errno, &peer);
    LOG_TRACE << "UdpServer::HandleRead length " << n;

    if (n < 0) {
        errno = saved_errno;
        LOG_SYSERR << "UdpServer::HandleRead";
        HandleError();
    } else if (n > 0) {
        if (message_callback_)
            message_callback_(shared_from_this(), &receive_buffer_, &peer,
                              poll_time);
    }
}

void UdpServer::HandleClose() { LOG_INFO << "UdpServer::HandleClose"; }

void UdpServer::HandleError() {
    int err = sockets::GetSocketErrno(channel_->fd());
    LOG_ERROR << "UdpServer::HandleError [" << name_ << "] - SO_ERROR = " << err
              << " " << strerror_tl(err);
}

} // namespace net
} // namespace muduo
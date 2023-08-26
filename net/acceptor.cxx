#include "acceptor.h"
#include "eventloop/channel.h"
#include "eventloop/event_loop.h"
#include "inet_address.h"
#include "logger/logger.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

namespace muduo {
namespace net {

Acceptor::Acceptor(event_loop::EventLoop *loop, const InetAddress &listen_addr,
                   bool reuseport)
    : loop_(loop),
      serv_address_(listen_addr),
      accept_socket_(sockets::CreateNonblockingOrDie(listen_addr.family())),
      accept_channel_(new event_loop::Channel(loop, accept_socket_.fd())),
      listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {

    assert(idle_fd_ >= 0);
    accept_socket_.SetReuseAddr(true);
    accept_socket_.SetReusePort(reuseport);
    accept_socket_.BindAddress(listen_addr);
    accept_channel_->set_read_callback(
        std::bind(&Acceptor::HandleRead, this, std::placeholders::_1));
}

Acceptor::~Acceptor() {
    accept_channel_->DisableAll();
    accept_channel_->RemoveFromLoop();
    ::close(idle_fd_);
}

void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    listening_ = true;
    accept_socket_.Listen();
    accept_channel_->EnableReading();
    LOG_DEBUG << this << "Listening on port " << serv_address_.Port();
}

void Acceptor::HandleRead(event_loop::Timestamp) {
    loop_->AssertInLoopThread();
    InetAddress peerAddr;
    // FIXME loop until no more
    int connfd = accept_socket_.Accept(&peerAddr);
    if (connfd >= 0) {
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;
        if (new_conn_cb_) {
            new_conn_cb_(connfd, peerAddr);
        } else {
            sockets::Close(connfd);
        }
    } else {
        LOG_SYSERR << "in Acceptor::HandleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace net
} // namespace muduo
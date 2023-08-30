#include "inet_socket.h"
#include "eventloop/endian.h"
#include "logger/logger.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/tcp.h>
#include <unistd.h>

namespace muduo {
namespace net {

Socket::~Socket() { sockets::Close(sock_fd_); }

void Socket::BindAddress(const InetAddress &addr) {
    sockets::BindOrDie(sock_fd_, addr.GetSockAddr());
}

void Socket::Listen() { sockets::ListenOrDie(sock_fd_); }

int Socket::Accept(InetAddress *peeraddr) {
    struct sockaddr_in6 addr;
    ::bzero(&addr, sizeof(addr));
    int connfd = sockets::Accept(sock_fd_, &addr);
    if (connfd >= 0) {
        peeraddr->SetSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::ShutdownWrite() {
    if (::shutdown(sock_fd_, SHUT_WR) < 0) {
        LOG_SYSERR << "Socket::ShutdownWrite";
    }
}

void Socket::SetTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, &optval,
                 static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::SetReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                 static_cast<socklen_t>(sizeof(optval)));
}

void Socket::SetReusePort(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                           static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
}

void Socket::SetKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                 static_cast<socklen_t>(sizeof optval));
}

namespace sockets {
int CreateNonblockingOrDie(sa_family_t family) {
    int fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                      IPPROTO_TCP);
    if (fd < 0) {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    return fd;
}

void BindOrDie(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr,
                     static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::BindOrDie";
    }
}

void ListenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int Accept(int sockfd, struct sockaddr_in6 *addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined(NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, (struct sockaddr *)(addr), &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno) {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOG_FATAL << "unexpected error of ::accept " << savedErrno;
            break;
        default:
            LOG_FATAL << "unknown error of ::accept " << savedErrno;
            break;
        }
    }
    return connfd;
}

ssize_t Write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void Close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_SYSERR << "sockets::close";
    }
}

void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = HostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_SYSERR << "sockets::FromIpPort";
    }
}

void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = HostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_SYSERR << "sockets::FromIpPort";
    }
}

int GetSocketErrno(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in6 GetLocalAddr(int sockfd) {
    struct sockaddr_in6 local_addr;
    ::bzero(&local_addr, sizeof(local_addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(local_addr));
    if (::getsockname(sockfd, (struct sockaddr *)(&local_addr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::GetLocalAddr";
    }
    return local_addr;
}

void ToIp(char *buf, size_t size, const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in *addr4 = (const struct sockaddr_in *)addr;
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf,
                    static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6 *)addr;
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf,
                    static_cast<socklen_t>(size));
    }
}

void ToIpPort(char *buf, size_t size, const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET6) {
        buf[0] = '[';
        ToIp(buf + 1, size - 1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6 *)addr;
        uint16_t port = NetworkToHost16(addr6->sin6_port);
        assert(size > end);
        snprintf(buf + end, size - end, "]:%u", port);
    } else {
        ToIp(buf, size, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in *addr4 = (const struct sockaddr_in *)addr;
        uint16_t port = NetworkToHost16(addr4->sin_port);
        assert(size > end);
        snprintf(buf + end, size - end, ":%u", port);
    }
}

} // namespace sockets
} // namespace net
} // namespace muduo
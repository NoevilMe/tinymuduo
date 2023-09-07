#ifndef C113F467_CAB2_43D4_B54D_5EC673EC66FB
#define C113F467_CAB2_43D4_B54D_5EC673EC66FB

#include "inet_address.h"

#include <netinet/in.h>

namespace muduo {
///
/// TCP networking.
///
namespace net {

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delagated to OS.
class Socket {
public:
    explicit Socket(int sockfd) : sock_fd_(sockfd) {}

    // Socket(Socket&&) // move constructor in C++11
    ~Socket();

    int fd() const { return sock_fd_; }
    // return true if success.
    // bool getTcpInfo(struct tcp_info *) const;
    // bool getTcpInfoString(char *buf, int len) const;

    /// abort if address in use
    void BindAddress(const InetAddress &addr);

    bool BindAddressAlive(const InetAddress &addr);

    /// abort if address in use
    void Listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int Accept(InetAddress *peeraddr);

    void ShutdownWrite();

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void SetTcpNoDelay(bool on);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void SetReuseAddr(bool on);

    ///
    /// Enable/disable SO_REUSEPORT
    ///
    void SetReusePort(bool on);

    ///
    /// Enable/disable SO_KEEPALIVE
    ///
    void SetKeepAlive(bool on);

private:
    const int sock_fd_;
};

namespace sockets {
///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int CreateNonblockingOrDie(sa_family_t family);

bool Bind(int sockfd, const struct sockaddr *addr);

// int connect(int sockfd, const struct sockaddr *addr);
void BindOrDie(int sockfd, const struct sockaddr *addr);
void ListenOrDie(int sockfd);

int Accept(int sockfd, struct sockaddr_in6 *addr);
// ssize_t read(int sockfd, void *buf, size_t count);
// ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

ssize_t Write(int sockfd, const void *buf, size_t count);
ssize_t SendTo(int sockfd, const void *buf, size_t count,
               const struct sockaddr *addr);

void Close(int sockfd);
// void shutdownWrite(int sockfd);

void ToIpPort(char *buf, size_t size, const struct sockaddr *addr);
void ToIp(char *buf, size_t size, const struct sockaddr *addr);

std::string ToIpPort(const struct sockaddr *addr);
std::string ToIp(const struct sockaddr *addr);

void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr);
void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in6 *addr);

int GetSocketErrno(int sockfd);

// const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
// const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
// struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);
// const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);
// const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);

struct sockaddr_in6 GetLocalAddr(int sockfd);
// struct sockaddr_in6 getPeerAddr(int sockfd);
// bool isSelfConnect(int sockfd);

///
/// Creates a non-blocking udp socket file descriptor,
int CreateNonblockingUdp(sa_family_t family);

} // namespace sockets
} // namespace net
} // namespace muduo

#endif /* C113F467_CAB2_43D4_B54D_5EC673EC66FB */

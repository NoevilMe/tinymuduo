#ifndef A5B387F9_5EE0_43E5_B146_6BF54ECD4C12
#define A5B387F9_5EE0_43E5_B146_6BF54ECD4C12

#include <string>

#include <netinet/in.h>

namespace muduo {
namespace net {

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, bool loopback_only = false,
                         bool ipv6 = false);

    InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);

    explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

    explicit InetAddress(const struct sockaddr_in6 &addr) : addr6_(addr) {}

    sa_family_t family() const { return addr_.sin_family; }

    std::string Ip() const;
    std::string IpPort() const;

    uint16_t Port() const;
    uint16_t PortNetEndian() const { return addr_.sin_port; }

    const struct sockaddr *GetSockAddr() const;

    void SetSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};

} // namespace net
} // namespace muduo

#endif /* A5B387F9_5EE0_43E5_B146_6BF54ECD4C12 */

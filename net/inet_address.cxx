#include "inet_address.h"
#include "eventloop/endian.h"
#include "inet_socket.h"
#include "mem.h"

#include <cstring>

namespace muduo {
namespace net {

InetAddress::InetAddress(uint16_t port, bool loopback_only, bool ipv6) {
    if (ipv6) {
        mem_zero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_addr = loopback_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_port = HostToNetwork16(port);

    } else {
        mem_zero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr =
            HostToNetwork32(loopback_only ? INADDR_LOOPBACK : INADDR_ANY);
        addr_.sin_port = HostToNetwork16(port);
    }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool ipv6) {
    if (ipv6 || ip.find(':') != std::string::npos) {
        mem_zero(&addr6_, sizeof(addr6_));
        sockets::FromIpPort(ip.data(), port, &addr6_);
    } else {
        mem_zero(&addr_, sizeof(addr_));
        sockets::FromIpPort(ip.data(), port, &addr_);
    }
}

const struct sockaddr *InetAddress::GetSockAddr() const {
    return (const struct sockaddr *)(&addr6_);
}

std::string InetAddress::Ip() const {
    char buf[64] = {0};
    sockets::ToIp(buf, sizeof(buf), GetSockAddr());
    return buf;
}

std::string InetAddress::IpPort() const {
    char buf[64] = {0};
    sockets::ToIpPort(buf, sizeof(buf), GetSockAddr());
    return buf;
}

uint16_t InetAddress::Port() const { return NetworkToHost16(PortNetEndian()); }

} // namespace net
} // namespace muduo
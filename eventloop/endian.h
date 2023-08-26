#ifndef B227EC26_F8E3_44BD_AF25_97DB462D27BA
#define B227EC26_F8E3_44BD_AF25_97DB462D27BA

#include <cstdint>
#include <endian.h>

namespace muduo {

#ifdef __WIN32
inline u_short networkToHost16(u_short netshort) { return ntohs(netshort); }

inline u_long networkToHost32(u_long netlong) { return ntohl(netlong); }

inline u_short hostToNetwork16(u_short netshort) { return htons(netshort); }

inline u_long hostToNetwork32(u_long netlong) { return htonl(netlong); }

inline unsigned __int64 networkToHost64(unsigned __int64 value) {
    return ntohll(value);
}

inline unsigned __int64 hostToNetwork64(unsigned __int64 value) {
    return htonll(value);
}
#else
// refer to https://linux.die.net/man/3/be16toh
inline uint16_t NetworkToHost16(uint16_t net16) { return be16toh(net16); }

inline uint32_t NetworkToHost32(uint32_t net32) { return be32toh(net32); }

inline uint64_t NetworkToHost64(uint64_t net64) { return be64toh(net64); }

inline uint16_t HostToNetwork16(uint16_t host16) { return htobe16(host16); }

inline uint32_t HostToNetwork32(uint32_t host32) { return htobe32(host32); }

inline uint64_t HostToNetwork64(uint64_t host64) { return htobe64(host64); }

#endif

} // namespace muduo

#endif /* B227EC26_F8E3_44BD_AF25_97DB462D27BA */

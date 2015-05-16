#ifndef _LIBAAN_BYTE_HH_
#define _LIBAAN_BYTE_HH_

#ifdef NO_GOOD
#include <winsock2.h>
#include <sys/param.h>

inline uint64_t htonll(uint64_t value)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    uint32_t high_part = htonl((uint32_t)(value >> 32));
    uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
    return (((uint64_t)low_part) << 32) | high_part;
#elif BYTE_ORDER == BIG_ENDIAN
    return value;
#endif
}

inline uint64_t ntohll(uint64_t n)
{
    return htonl(1) == 1 ? n : ((uint64_t)ntohl(n) << 32) | ntohl(n >> 32);
}

#if BYTE_ORDER == LITTLE_ENDIAN
#define htobe64(x) htonll(x)
#define htole64(x) (x)
#define be64toh(x) ntohll(x)
#define le64toh(x) (x)
#elif BYTE_ORDER == BIG_ENDIAN
#define htobe64(x) (x)
#define htole64(x) __builtin_bswap64(x)
#define be64toh(x) (x)
#define le64toh(x) __builtin_bswap64(x)
#endif
#endif

#include <cstddef>

namespace libaan {

inline std::size_t roundtonext8(std::size_t val) { return (val + 7) & ~7; }
inline std::size_t roundtolast8(std::size_t val) { return val & ~7; }

}

#endif


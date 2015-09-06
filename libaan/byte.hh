#ifndef _LIBAAN_BYTE_HH_
#define _LIBAAN_BYTE_HH_

#include <cstdint>
#include <type_traits>

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

inline std::size_t roundtonext8(std::size_t val) { return (val + 7ull) & ~7ull; }
inline std::size_t roundtolast8(std::size_t val) { return val & ~7ull; }
inline std::size_t roundtonext16(std::size_t val) { return (val + 15ull) & ~15ull; }
inline std::size_t roundtolast16(std::size_t val) { return val & ~15ull; }


template<size_t start, size_t end>
inline bool in_region(size_t val)
{
    return val >= start && val <= end;
}

/*
 — Built-in Function: int __builtin_clz (unsigned int x)

    Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined. 

— Built-in Function: int __builtin_ctz (unsigned int x)

    Returns the number of trailing 0-bits in x, starting at the least significant bit position. If x is 0, the result is undefined. 


*/

// TODO: use std::numeric_limits<T>::digits


// 1111000 := 4 most-significant bits are 1 -> 4 leading 1-bits, 4 trailing 0-bits

template<typename value_type>
inline size_t count_leading_0(value_type value)
{
    static_assert(sizeof(value_type) == 4, "not 32bit");
    return value == 0 ? sizeof(value) * 8 : __builtin_clz(value);
}

inline size_t count_leading_0(uint64_t value)
{
    return value == 0 ? sizeof(value) * 8
        : ((value >> 32) == 0 ? (32 + __builtin_clz(value & 0xffffffffULL)) : 0);
}

template<typename value_type>
inline size_t count_trailing_0(value_type value)
{
    static_assert(sizeof(value_type) == 4, "not 32bit");
    return value == 0 ? sizeof(value) * 8 : __builtin_ctz(value);
}

inline size_t count_trailing_0(uint64_t value)
{
    return value == 0 ? sizeof(value) * 8
        : (((value & 0xffffffffULL) == 0) ? 32 + __builtin_ctz(value >> 32) : __builtin_ctz(value & 0xffffffffULL));
}

// TODO: testing value for 0 is unnecessary since it would be all-1 after inverting
template<typename value_type>
inline size_t count_leading_1(value_type value)
{
    static_assert(sizeof(value_type) == 4, "not 32bit");
    return value == 0 ? 0 : (~value == 0 ? 8 * sizeof(value_type) : __builtin_clz(~value));
}

inline size_t count_leading_1(uint64_t value)
{
    if(value == 0)
        return 0;
    else if(~value == 0ULL)
        return 64;

    // __builtin_clz(0) results in undefined behaviour
    const auto msb = (~value >> 32) == 0 ? 32 : __builtin_clz(~value >> 32);
    const auto lsb = (~value & 0xffffffffUL) == 0 ? 32 : __builtin_clz(~value & 0xffffffffUL);
    return msb == 32 ? 32 + lsb : msb;
}

template<typename value_type>
inline size_t count_trailing_1(value_type value)
{
    static_assert(sizeof(value_type) == 4, "not 32bit");
    return value == 0 ? 0 : (~value == 0 ? 8 * sizeof(value_type) : __builtin_ctz(~value));
}

inline size_t count_trailing_1(uint64_t value)
{
    if(value == 0)
        return 0;
    else if(~value == 0ULL)
        return 64;

    // __builtin_clz(0) results in undefined behaviour
    const auto msb = (~value >> 32) == 0 ? 32 : __builtin_ctz(~value >> 32);
    const auto lsb = (~value & 0xffffffffUL) == 0 ? 32 : __builtin_ctz(~value & 0xffffffffUL);
    return lsb == 32 ? 32 + msb : lsb;
}

/*
template<typename value_type>
uint32_t pad32_trailing_0(value_type value)
{
    const auto sz = 4 < sizeof(value) ? 0 : (4 - sizeof(value));
    return sz == 0 ? value : (sz == 1 ? value << 8 : (sz == 2 ? value << 16 : value << 24));
}
*/

}

#endif


#pragma once

#include <arpa/inet.h>
#include <bitset>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <type_traits>

namespace libaan {
sockaddr_storage convert(const sockaddr &addr);

std::string ntop(const sockaddr_storage &addr);
bool pton4(const std::string &addr_string, in_addr &addr);
bool pton6(const std::string &addr_string, in6_addr &addr);
bool pton(int af, const std::string &addr_string, sockaddr_storage &addr);

template<typename T> inline std::string to_hex_string(T value);
template<typename T> std::string dump_bin(T val);
std::string ai_family_to_string(int ai_family);
std::string error_string(int err);

void strip(std::string &str);
}


inline void libaan::strip(std::string &str)
{
    int i = str.size();
    for(; i >= 0; i--)
        if(str[i] != '\0')
            break;
    str.resize(i + 1);
}

inline sockaddr_storage libaan::convert(const sockaddr &addr)
{
    sockaddr_storage dst;
    std::memset(&dst, 0, sizeof(dst));
    if(addr.sa_family == AF_INET) {
        auto dstp = reinterpret_cast<void *>(&reinterpret_cast<sockaddr_in *>(&dst)->sin_addr);
        std::memcpy(dstp, reinterpret_cast<const void *>(&reinterpret_cast<const sockaddr_in *>(&addr)->sin_addr),
                    sizeof(sockaddr_in));
        dst.ss_family = addr.sa_family;
    } else if(addr.sa_family == AF_INET6) {
        auto dstp = reinterpret_cast<void *>(&reinterpret_cast<sockaddr_in6 *>(&dst)->sin6_addr);
        std::memcpy(dstp, reinterpret_cast<const void *>(&reinterpret_cast<const sockaddr_in6 *>(&addr)->sin6_addr),
                    sizeof(sockaddr_in6));
        dst.ss_family = addr.sa_family;
    }
    return dst;
}

inline std::string libaan::ntop(const sockaddr_storage &addr)
{
    std::string buf;
    switch(addr.ss_family) {
    case AF_INET:
        buf.resize(INET_ADDRSTRLEN, '\0');
        inet_ntop(addr.ss_family,
                  reinterpret_cast<const void *>(
                      &reinterpret_cast<const sockaddr_in *>(&addr)->sin_addr),
                  &buf[0], buf.length());
        break;
    case AF_INET6:
        buf.resize(INET6_ADDRSTRLEN);
        inet_ntop(addr.ss_family,
                  reinterpret_cast<const void *>(
                      &reinterpret_cast<const sockaddr_in6 *>(&addr)->sin6_addr),
                  &buf[0], buf.length());
        break;
    default:
        return "";
    }

    strip(buf);
    return buf;
}

inline bool libaan::pton4(const std::string &addr_string, in_addr &addr)
{
    return inet_pton(AF_INET, addr_string.c_str(), reinterpret_cast<void *>(&addr)) == 1;
}

inline bool libaan::pton6(const std::string &addr_string, in6_addr &addr)
{
    return inet_pton(AF_INET6, addr_string.c_str(), reinterpret_cast<void *>(&addr)) == 1;
}

inline bool libaan::pton(int af, const std::string &addr_string, sockaddr_storage &addr)
{
    std::memset(&addr, 0, sizeof(addr));
    switch(af) {
    case AF_INET:
        addr.ss_family = af;
        return pton4(addr_string.c_str(), reinterpret_cast<sockaddr_in *>(&addr)->sin_addr);
    case AF_INET6:
        addr.ss_family = af;
        return pton6(addr_string.c_str(), reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_addr);
    default:
        addr.ss_family = AF_UNSPEC;
        return false;
    }
}

template<typename T> inline std::string libaan::to_hex_string(T value)
{
    std::ostringstream os;
    os << std::hex << value << std::dec;
    return os.str();
}

template<typename T> inline std::string libaan::dump_bin(T val)
{
    // forbidden:
    //ipv6_util::sa6_8_t x; libaan::dump_bin(x);
    //std::string x2; libaan::dump_bin(x2);
    static_assert(std::is_pod<T>::value && !std::is_compound<T>::value && !std::is_array<T>::value,
                  "dump_bin only supports POD types.");

    std::bitset<sizeof(T) * 8> bin(val);
    std::ostringstream os;
    os << bin;
    return os.str();
}

inline std::string libaan::error_string(int err)
{
    std::string buff(256, 0);
    auto ret = strerror_r(err, &buff[0], buff.size());
    if(ret != &buff[0])
        buff.assign(ret);
    return buff;
}

inline std::string libaan::ai_family_to_string(int ai_family)
{
    if(ai_family == AF_INET)
        return "AF_INET";
    else if(ai_family == AF_INET6)
        return "AF_INET6";
    else if(ai_family == AF_UNSPEC)
        return "AF_UNSPEC";
    else if(ai_family == AF_PACKET)
        return "AF_PACKET";
    return "";
}

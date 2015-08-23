#pragma once

#include <cstring>
#include <netinet/in.h>
#include "byte.hh"

namespace libaan {

bool is_ipv4(const sockaddr_storage &sa);
const in_addr &get_inaddr(const sockaddr_storage &sa);
in_addr &get_inaddr(sockaddr_storage &sa);
sockaddr_storage init4(const in_addr &a);

bool is_in4addr_loopback(const in_addr &a);
bool is_in4addr_any(const in_addr &a);
bool is_in4addr_broadcast(const in_addr &a);
bool is_in4addr_multicast(const in_addr &a);
bool is_in4addr_private(const in_addr &a);

bool is_in4addr_multicast_unspec(const in_addr &a);
bool is_in4addr_multicast_allhosts(const in_addr &a);
bool is_in4addr_multicast_allrouters(const in_addr &a);
bool is_in4addr_multicast_maxlocalgroup(const in_addr &a);

bool is_in4addr_class_a(const in_addr &a);
bool is_in4addr_class_b(const in_addr &a);
bool is_in4addr_class_c(const in_addr &a);
bool is_in4addr_class_d(const in_addr &a);

bool is_in4addr_linklocal(const in_addr &a);
bool is_in4addr_special_reserved(const in_addr &a);

bool is_in4addr_reserved(const in_addr &a);
// is address not reserved and not subnet all-1/all-0 address.
bool is_in4addr_notreserved_valid(const in_addr &a, size_t prefix);

bool is_in4addr_subnet_address(const in_addr &a, size_t prefix);
bool is_in4addr_subnet_broadcastaddress(const in_addr &a, size_t prefix);

size_t count_leading_1(const in_addr &a);
size_t count_trailing_1(const in_addr &a);
in_addr get_netmask(size_t prefix);

}

inline bool libaan::is_ipv4(const sockaddr_storage &sa)
{
    return sa.ss_family == AF_INET;
}

inline const in_addr &libaan::get_inaddr(const sockaddr_storage &sa)
{
    return reinterpret_cast<const sockaddr_in *>(&sa)->sin_addr;
}

inline in_addr &libaan::get_inaddr(sockaddr_storage &sa)
{
    return reinterpret_cast<sockaddr_in *>(&sa)->sin_addr;
}

inline sockaddr_storage libaan::init4(const in_addr &a)
{
    sockaddr_storage sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.ss_family = AF_INET;
    auto &dst = get_inaddr(sa);
    dst = a;
    return sa;
}

inline bool libaan::is_in4addr_loopback(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_LOOPBACK);
}

inline bool libaan::is_in4addr_multicast(const in_addr &a)
{
    return IN_MULTICAST(htonl(a.s_addr));
}

inline bool libaan::is_in4addr_any(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_ANY);
}

inline bool libaan::is_in4addr_broadcast(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_BROADCAST);
}

inline bool libaan::is_in4addr_multicast_unspec(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_UNSPEC_GROUP);
}

inline bool libaan::is_in4addr_multicast_allhosts(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_ALLHOSTS_GROUP);
}

inline bool libaan::is_in4addr_multicast_allrouters(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_ALLRTRS_GROUP);
}

inline bool libaan::is_in4addr_multicast_maxlocalgroup(const in_addr &a)
{
    return a.s_addr == ntohl(INADDR_MAX_LOCAL_GROUP);
}

inline bool libaan::is_in4addr_class_a(const in_addr &a)
{
    return IN_CLASSA(htonl(a.s_addr));
}

inline bool libaan::is_in4addr_class_b(const in_addr &a)
{
    return IN_CLASSB(htonl(a.s_addr));
}

inline bool libaan::is_in4addr_class_c(const in_addr &a)
{
    return IN_CLASSC(htonl(a.s_addr));
}

inline bool libaan::is_in4addr_class_d(const in_addr &a)
{
    return IN_CLASSD(htonl(a.s_addr));
}


inline bool libaan::is_in4addr_linklocal(const in_addr &a)
{
    // 169.254.0.0/16
    return (a.s_addr & htonl(0xffff0000)) == htonl(0xa9fe0000);
}

inline bool libaan::is_in4addr_special_reserved(const in_addr &a)
{
    return
        // 0.0.0.0/8
        (a.s_addr & htonl(0xff000000)) == 0
        // 100.64.0.0/10
        || (a.s_addr & htonl(0xffc00000)) == htonl(0x64400000)
        // 192.0.0.0/24
        || (a.s_addr & htonl(0xffffff00)) == htonl(0xc0000000)
        // 192.0.2.0/24
        || (a.s_addr & htonl(0xffffff00)) == htonl(0xc0000200)
        // 192.88.99.0/24
        || (a.s_addr & htonl(0xffffff00)) == htonl(0xc0586300)
        // 198.18.0.0/15
        || (a.s_addr & htonl(0xfffe0000)) == htonl(0xc6120000)
        // 198.51.100.0/24
        || (a.s_addr & htonl(0xffffff00)) == htonl(0xc6336400)
        // 203.0.113.0/24
        || (a.s_addr & htonl(0xffffff00)) == htonl(0xcb007100)
        // 240.0.0.0/4
        || (a.s_addr & htonl(0xf0000000)) == htonl(0xf0000000);
}

inline bool libaan::is_in4addr_subnet_address(const in_addr &a, size_t prefix)
{
    const auto hostmask = ~get_netmask(prefix).s_addr;
    return (hostmask & a.s_addr) == 0;
}

inline bool libaan::is_in4addr_subnet_broadcastaddress(const in_addr &a, size_t prefix)
{
    const auto hostmask = ~get_netmask(prefix).s_addr;
    // tmp contains host part of a
    in_addr tmp;
    tmp.s_addr = hostmask & a.s_addr;

    return (count_trailing_1(tmp) == 32 - prefix);
}

inline bool libaan::is_in4addr_notreserved_valid(const in_addr &a, size_t prefix)
{
    return !is_in4addr_special_reserved(a)
        && !is_in4addr_subnet_address(a, prefix)
        && !is_in4addr_subnet_broadcastaddress(a, prefix);
}

inline bool libaan::is_in4addr_reserved(const in_addr &a)
{
    return is_in4addr_loopback(a)
        || is_in4addr_any(a)
        || is_in4addr_broadcast(a)
        || is_in4addr_multicast(a)
        || is_in4addr_private(a)
        || is_in4addr_linklocal(a)
        || is_in4addr_special_reserved(a);
}

/*
rfc1918
     10.0.0.0        -   10.255.255.255  (10/8 prefix)
     172.16.0.0      -   172.31.255.255  (172.16/12 prefix)
     192.168.0.0     -   192.168.255.255 (192.168/16 prefix)
*/
inline bool libaan::is_in4addr_private(const in_addr &a)
{
    if((a.s_addr & htonl(0xff000000)) == htonl(0x0a000000))
        return true;

    if((a.s_addr & htonl(0xffff0000)) == htonl(0xc0a80000))
        return true;

    if((a.s_addr & htonl(0xfff00000)) == htonl(0xac100000))
        return true;

    return false;
}

/*
inline in_addr cidr_to_netmask(size_t cidr_prefix)
{

}

inline size_t netmask_to_cidr(const in_addr &netmask)
{

}

inline bool match_subnets(const in_addr &subnet, const in_addr &ip, const in_addr &netmask)
{
    return (ip & netmask) == (subnet & netmask);
}

inline in_addr calculate_subnet_address(const in_addr &broadaddr, const in_addr_t &netmask)
{
    return (broadaddr & netmask);
}
*/

inline size_t libaan::count_leading_1(const in_addr &a)
{
    return bit_util::count_leading_1(ntohl(a.s_addr));
}

inline size_t libaan::count_trailing_1(const in_addr &a)
{
    return bit_util::count_trailing_1(ntohl(a.s_addr));
}

inline in_addr libaan::get_netmask(size_t prefix)
{
    static const std::array<uint32_t, 33> le_mask32 = {
        0x00000000, 0x80000000, 0xc0000000, 0xe0000000,
        0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000,

        0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
        0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000,

        0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000,
        0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,

        0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
        0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe,
        0xffffffff,
    };
    in_addr a;
    a.s_addr = htonl(le_mask32[prefix % 32]);
    return a;
}

#endif


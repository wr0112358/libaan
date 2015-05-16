#ifndef _LIBAAN_DEBUG_HH_
#define _LIBAAN_DEBUG_HH_

#include <bitset>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace libaan {

void do_ls(const std::string &path, std::ostream &out = std::cout);

// Src must have an even number of [0-9a-f] characters.
// In the uneven case it returns false and converted input.
std::pair<bool, std::vector<unsigned char> > hex2bin(const char *src, size_t size_hint = 0);
std::string bin2hex(const std::vector<unsigned char> &v);

void strip(std::string &str);

template<typename T>
std::string dump_bin(T val)
{
    // forbidden:
    //ipv6_util::sa6_8_t x; ip_util::dump_bin(x);
    //std::string x2; ip_util::dump_bin(x2);
    static_assert(std::is_pod<T>::value && !std::is_compound<T>::value && !std::is_array<T>::value,
                  "dump_bin only supports POD types.");

    std::bitset<sizeof(T) * 8> bin(val);
    std::ostringstream os;
    os << bin;
    return os.str();
}

}

#endif

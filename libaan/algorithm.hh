#ifndef _LIBAAN_ALGORITHM_HH_
#define _LIBAAN_ALGORITHM_HH__

#include <algorithm>
#include <cstring>
#include <string>

//#include <iostream>

namespace libaan {

// 0*<suffix> -> <suffix>
inline void bin_remove_leading_0(std::string &str)
{
    const auto idx = str.find_first_not_of('0');
    if(idx != std::string::npos)
        str.erase(0, idx);
}

/*
0^3<suffix> -> 0^3<suffix>
0^8<suffix> -> <suffix>
0^9<suffix> -> 0<suffix>
0^15<suffix> -> 0^7<suffix>
0^16<suffix> -> <suffix>
*/
inline void bin_remove_leading_0_full_byte(std::string &str)
{
    const auto idx = str.find_first_not_of('0');
    if(idx == std::string::npos)
        return;
    const auto round_down = idx & ~0x07u;
    if(round_down)
        str.erase(0, round_down);
}

inline std::string error_string(int err)
{
    std::string buff(256, 0);
    auto ret = strerror_r(err, &buff[0], buff.size());
    if(ret != &buff[0])
        buff.assign(ret);
    return buff;
}

template<typename container_t>
bool all_eq(const container_t &c)
{
    return std::adjacent_find(std::begin(c), std::end(c),
                              std::not_equal_to<typename container_t::value_type>())
        == std::end(c);
}

template<typename container_t>
bool all_eq2(const container_t &c)
{
    return c.empty()
        ? true
        : std::all_of(std::begin(c), std::end(c),
                      [&c](const typename container_t::value_type &v) {
                          return v == *std::begin(c); });
}

template<typename it_a, typename it_b>
it_a find_first_not_of(const it_a &begin_a, const it_a &end_a,
                       const it_b &begin_b, const it_b &end_b)
{
    for(auto mid_a = begin_a; mid_a != end_a; ++mid_a) {
        for(auto mid_b = begin_b; mid_b != end_b; ++mid_b)
            if(*mid_a == *mid_b)
                goto continue_outer;
        return mid_a;
    continue_outer:
        ;
    }
    return end_a;
}

template<typename container_t>
bool contains_only(const container_t &c, const container_t &set)
{
    return find_first_not_of(std::begin(c), std::end(c),
                             std::begin(set), std::end(set))
        == std::end(c);
}

//template<typename container_t>
//bool contains_only(const typename std::enable_if<std::is_same<std::string,
//                   container_t>::value >::type  &c,
//                   const container_t &set)
inline bool contains_only(const std::string &c, const std::string &set)
{
    return c.find_first_not_of(set) == std::string::npos;
}

template<typename T>
const T& min(const T &a, const T &b, const T &c)
{ return std::min(std::min(a, b), c); }

template<typename container_type>
using value_t = typename container_type::value_type;

template<typename container_type>
value_t<container_type> mean(const container_type &v)
{
    if(v.empty())
        return value_t<container_type>();
    auto sum = std::accumulate(std::begin(v), std::end(v), value_t<container_type>());
    return sum / v.size();
}

template<typename container_type>
value_t<container_type> min(const container_type &v)
{
    return v.empty()
        ? value_t<container_type>()
        : *std::min_element(std::begin(v), std::end(v));
}

template<typename container_type>
value_t<container_type> max(const container_type &v)
{
    return v.empty()
        ? value_t<container_type>()
        : *std::max_element(std::begin(v), std::end(v));
}

inline bool ends_with(const void *haystack, size_t haystack_len,
               const void *needle, size_t needle_len)
{
    return haystack_len >= needle_len
        && memcmp(reinterpret_cast<const char *>(haystack) + haystack_len - needle_len, needle, needle_len) == 0;
}

// returns 0 if haystack contains the complete needle
// returns 1 if haystack contains the complete needle - 1
// returns 2 if haystack contains the complete needle - 2
// ...
// returns needle_len if needle is not found
inline size_t ends_with_prefix(const void *haystack, size_t haystack_len,
                        const void *needle, size_t needle_len)
{
    for(size_t i = 0; i < needle_len; i++) {
        //std::cout << "prefix: " << std::string((char *)needle, needle_len - i) << "\n";
        //std::cout << "hay: " << std::string((char *)haystack, haystack_len) << "\n";
        if(ends_with(haystack, haystack_len,
                     needle, needle_len - i))
            return i;
    }
    return needle_len;
}

inline bool starts_with(const void *haystack, size_t haystack_len,
                 const void *needle, size_t needle_len)
{
    return haystack_len >= needle_len
        && memcmp(haystack, needle, needle_len) == 0;
}

// returns 0 if haystack contains the complete needle
// returns 1 if haystack contains the complete needle - 1
// returns 2 if haystack contains the complete needle - 2
// ...
// returns needle_len if needle is not found
inline size_t starts_with_suffix(const void *haystack, size_t haystack_len,
                          const void *needle, size_t needle_len)
{
    for(size_t i = 0; i < needle_len; i++) {
        //std::cout << "suffix: " << std::string((char *)needle + i, needle_len - i) << "\n";
        //std::cout << "hay: " << std::string((char *)haystack, haystack_len) << "\n";
        if(starts_with(haystack, haystack_len,
                       reinterpret_cast<const char *>(needle) + i,
                       needle_len - i))
            return i;
    }
    return needle_len;
}

}

#endif

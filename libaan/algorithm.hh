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
    const auto round_down = idx & ~0x07;
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

}

template<typename T>
const T& min(const T &a, const T &b, const T &c)
{ return std::min(std::min(a, b), c); }

#endif

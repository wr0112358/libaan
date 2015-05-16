#ifndef _LIBAAN_TIME_HH_
#define _LIBAAN_TIME_HH_

#include <chrono>
#include <string>

#include "byte.hh"

namespace libaan {

using monotonic_clock_t = std::chrono::steady_clock;
using clock_t = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<clock_t>;
using monotonic_time_point_t = std::chrono::time_point<monotonic_clock_t>;

// not for steady_clock
inline std::string
to_string(const time_point_t &t, bool filename = true, const char *format = nullptr);

template<typename clock_resolution, typename time_point>
inline std::string dura_to_string(const time_point &start, const time_point &end = time_point::clock::now());


template<typename time_point>
inline int64_t storable_time_point(const time_point &t);

template<typename time_point>
inline std::string storable_time_point_bin(const time_point &t);

template<typename time_point>
inline const time_point deserialize_time_point(int64_t t);

template<typename time_point>
inline const time_point deserialize_time_point(const std::string &t);

template<typename time_point>
inline std::string storable_time_point_now_bin();

template<typename time_point>
inline int64_t storable_time_point_now();

template<typename time_point, typename clock_resolution>
class timer {
public:
    timer() : start(time_point::clock::now()) {}

    // returns duration in the specified clock resolution: s/ms/us/ns
    inline double duration(const time_point *end_time = nullptr)
    {
        const auto t = end_time ? *end_time : time_point::clock::now();
        return std::chrono::duration_cast<clock_resolution>(t - start).count();
    }

    const time_point start;
};

using timer_s = timer<monotonic_time_point_t, std::chrono::seconds>;
using timer_ms = timer<monotonic_time_point_t, std::chrono::milliseconds>;
using timer_us = timer<monotonic_time_point_t, std::chrono::microseconds>;
using timer_ns = timer<monotonic_time_point_t, std::chrono::nanoseconds>;

}

std::string
libaan::to_string(const time_point_t &t, bool filename, const char *format)
{
    const auto ti = std::chrono::system_clock::to_time_t(t);

    const auto format_string = format ? format
        : (filename ? "%d.%m.%Y_%H.%M.%S" : "%d.%m.%Y %H:%M:%S");

    std::string mbstr(100, '\0');
    mbstr.resize(std::strftime(&mbstr[0], mbstr.length(), format_string,
                               std::localtime(&ti)));
    return mbstr;
}

template<typename clock_resolution, typename time_point>
inline std::string
libaan::dura_to_string(const time_point &start, const time_point &end)
{
    const auto delta = end - start;
    return std::to_string(std::chrono::duration_cast<clock_resolution>(delta).count());
}

template<typename time_point>
inline int64_t libaan::storable_time_point(const time_point &t)
{
    using duration64_type = std::chrono::duration<int64_t, std::nano>;
    return htobe64(std::chrono::duration_cast<duration64_type>(t.time_since_epoch()).count());
}

template<typename time_point>
inline std::string libaan::storable_time_point_bin(const time_point &t)
{
    int64_t time_int = storable_time_point(t);

    std::string time_string(8, 0);
    std::copy(reinterpret_cast<char *>(&time_int),
              reinterpret_cast<char *>(&time_int) + sizeof(int64_t),
              &time_string[0]);

    return time_string;
}

template<typename time_point>
inline const time_point libaan::deserialize_time_point(int64_t t)
{
    using duration64_type = std::chrono::duration<int64_t, std::nano>;
    return time_point(duration64_type(be64toh(t)));
}

template<typename time_point>
inline const time_point libaan::deserialize_time_point(const std::string &t)
{
    //assert(t.length() == sizeof(int64_t));
    int64_t time = 0;
    std::copy(&t[0], &t[0] + sizeof(int64_t), (char *)&time);
    return deserialize_time_point<time_point>(time);
}

template<typename time_point>
inline std::string libaan::storable_time_point_now_bin()
{
    return storable_time_point_bin(time_point::clock::now());
}

template<typename time_point>
inline int64_t libaan::storable_time_point_now()
{
    return storable_time_point(time_point::clock::now());
}

#endif

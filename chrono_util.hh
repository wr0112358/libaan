/*
Copyright (C) 2014 Reiter Wolfgang wr0112358@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _LIBAAN_CHRONO_UTIL_HH_
#define _LIBAAN_CHRONO_UTIL_HH_

#include <chrono>
#include <string>

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
// rather senseless, but..
#define htobe64(x) (x)
#define htole64(x) __builtin_bswap64(x)
#define be64toh(x) (x)
#define le64toh(x) __builtin_bswap64(x)
#endif
#endif

namespace libaan
{
namespace util
{
template <typename clock_resolution> class time_me
{
    public:
    // TODO: need pause/continue functionality. add elapsed times to a duration
    // time?
    // TODO: need stop() function -> calculate time duration
    time_me() { update_time(); }
    void restart() { update_time(); }

    // returns duration in the specified clock resolution: s/ms/us/ns
    inline double duration()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<clock_resolution>(now - start)
            .count();
    }

    private:
    inline void update_time()
    {
        start = std::chrono::high_resolution_clock::now();
    }

    private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

using time_me_s = time_me<std::chrono::seconds>;
using time_me_ms = time_me<std::chrono::milliseconds>;
using time_me_us = time_me<std::chrono::microseconds>;
using time_me_ns = time_me<std::chrono::nanoseconds>;

inline std::string
to_string(const std::chrono::time_point<std::chrono::high_resolution_clock> &t,
          bool filename = true)
{
    const auto ti = std::chrono::system_clock::to_time_t(t);
    char mbstr[100];
    if(filename) {
        if(!std::strftime(mbstr, sizeof(mbstr), "%m.%d.%Y_%H.%M.%S",
                          std::localtime(&ti)))
            return "";
    } else {
        if(!std::strftime(mbstr, sizeof(mbstr), "%m.%d.%Y %H:%M:%S",
                          std::localtime(&ti)))
            return "";
    }

    return std::string(mbstr);
}

inline std::string storable_time_point(
    const std::chrono::time_point<std::chrono::high_resolution_clock> &t)
{
    using duration64_type = std::chrono::duration<int64_t>;
    // without the cast to duration64_type, the timepoint cannot be reconstructed.
    const auto t_count = std::chrono::duration_cast<duration64_type>(t.time_since_epoch()).count();
    const int64_t time_int = htobe64(t_count);

    std::string time_string(8, 0);
    std::copy(reinterpret_cast<char *>(const_cast<int64_t *>(&time_int)),
              reinterpret_cast<char *>(const_cast<int64_t *>(&time_int))
              + sizeof(int64_t),
              &time_string[0]);

    return time_string;
}

inline const std::chrono::time_point<std::chrono::high_resolution_clock>
storable_time_point(const std::string &t)
{
    int64_t time = 0;
    if(t.length() == sizeof(int64_t)) {
        std::copy(&t[0], &t[0] + sizeof(int64_t), (char *)&time);
        // network byte order to host byte order
        time = be64toh(time);
    }

    const std::chrono::duration<int64_t>::rep rep(time);
    const std::chrono::duration<int64_t> duration(rep);
    const std::chrono::time_point<std::chrono::high_resolution_clock> timepoint(duration);

    return timepoint;
}

inline std::string storable_time_point_now()
{
    return storable_time_point(std::chrono::high_resolution_clock::now());
}

}
}

#endif

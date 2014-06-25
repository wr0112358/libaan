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
to_string(const std::chrono::time_point<std::chrono::high_resolution_clock> &t)
{
    const auto ti = std::chrono::system_clock::to_time_t(t);
    char mbstr[100];
    if(!std::strftime(mbstr, sizeof(mbstr), "%m.%d.%Y_%H.%M.%S",
                      std::localtime(&ti)))
        return "";
    return std::string(mbstr);
}
}
}

#endif

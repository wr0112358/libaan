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

#ifndef _LIBAAN_STRING_UTIL_HH_
#define _LIBAAN_STRING_UTIL_HH_

#include <string>

namespace libaan
{
namespace util
{
// a better aproach using a writeable input string:
// - replace first char of every delim with '\0'
// - save pointer to first char of everytoken in vector<char *>
// or with non writeable input:
// - save pointer to first char of every token together with string length in
//   vector<pair<char *, int> >
// need to test the timings
inline std::vector<std::string> split(const std::string &input,
                                      const std::string &delim)
{
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end;

    for(;;) {
        end = input.find(delim, start);
        tokens.push_back(input.substr(start, end - start));
        // We just copied the last token
        if(end == std::string::npos)
            break;
        // Exclude the delimiter in the next search
        start = end + delim.size();
    }

    return tokens;
}

inline std::vector<std::pair<const char *, size_t> >
split2(const std::string &input, const std::string &delim)
{
    std::vector<std::pair<const char *, size_t> > tokens;
    std::string::size_type start = 0;
    std::string::size_type end;

    for(;;) {
        end = input.find(delim, start);

        // We just copied the last token
        if(end == std::string::npos) {
            tokens.push_back(
                std::make_pair(&input.data()[start], input.size() - start));
            break;
        }

        tokens.push_back(std::make_pair(&input.data()[start], end - start));
        // Exclude the delimiter in the next search
        start = end + delim.size();
    }

    return tokens;
}
}
}

#endif

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

#ifdef TOTAL_REGEX_OVERLOAD
#include <regex>
#endif
#include <cstring>
#include <string>
#include <vector>

namespace libaan
{
namespace util
{

using string_type = std::pair<const char *, size_t>;

// a more performant aproach is split2 which avoids copies.
// for timings: test/test_string_util.cc
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


inline std::string tostring(const libaan::util::string_type &s)
{
    return std::string(s.first, s.first + s.second);
}

inline std::vector<string_type>
split2(const std::string &input, const std::string &delim, bool skip_empty = false)
{
    std::vector<string_type> tokens;
    std::string::size_type start = 0;
    std::string::size_type end;

    for(;;) {
        end = input.find(delim, start);
        if(skip_empty && (end - start) == 0) {
            start = end + delim.size();
            continue;
        }

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

// split an input string_type provided by a previous split
inline std::vector<string_type>
split2(const string_type &input, unsigned char delim, bool skip_empty = false)
{
    std::vector<string_type> ret;
    const char *ptr = input.first;
    size_t len = input.second;
    do {
        const char *found = reinterpret_cast<const char *>(std::memchr(ptr, delim, len));
        if(!found || !(found + 1)) {
            if(len || !skip_empty)
                ret.push_back(std::make_pair(ptr, len));
            return ret;
        } else {
            const auto tmp = found - ptr;
            if(tmp || !skip_empty)
                ret.push_back(std::make_pair(ptr, tmp));
            ptr = ++found;
            if(len)
                len -= tmp ? tmp + 1 : 1;
        }
    } while(true);
    return ret;
}

/*
string_type find(const string_type &haystack, const std::string &needle)
{
    // equal to std::end(*haystack) ? need to test
    const auto end = haystack.first + haystack.second
    const auto found = std::search(haystack.first, end,
                       std::begin(needle), std::end(needle));
    return found;
}
*/

#ifdef TOTAL_REGEX_OVERLOAD
// no working implementation available(?). programming against the standard
// is no fun.
/* example usage:
    const std::string path = "/proc/" + std::to_string(pid) + "/environ";
    std::string file_buffer;
    const auto size = read_file(path, file_buffer);
    std::cout << "read " << size << "bytes from " << path << "\n";

#ifdef TOTAL_REGEX_OVERLOAD
    const auto tokens = split(file_buffer, std::regex(R"(('.'|'\\0'))"));
#endif

    for(const auto & token: tokens) {
        const std::string PREFIX = "PWD=";
        if(string_starts_with(token, PREFIX)) {
            std::cout << "PWD-token = \"" << token << "\"\n";
            return token.substr(PREFIX.length(), token.length());
        }
    }
*/
inline std::vector<std::string> split(const std::string &input,
                                      const std::regex &regex)
{
    // passing -1 as the submatch index parameter performs splitting
    std::sregex_token_iterator first{input.begin(), input.end(), regex, -1};
    std::sregex_token_iterator last;
    return {first, last};
}
#endif


}
}

#endif

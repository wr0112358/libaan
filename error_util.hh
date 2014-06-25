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

#ifndef _LIBAAN_ERROR_UTIL_HH_
#define _LIBAAN_ERROR_UTIL_HH_

#include <system_error>

namespace libaan
{
namespace util
{

/* Usage:

   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>

   auto e = err(open, "/root/abc", O_CREAT);
   if(e.code())
   std::cout << e.code() << ": " << e.what() << std::endl;
*/

template <typename func, typename... args>
std::system_error check(func f, args... a) noexcept
{
    if (f(a...) == -1)
        return std::system_error(errno, std::system_category());
    else
        return std::system_error(0, std::system_category());
}

// TODO: use std::is_integral to test return_type.
template <typename func, typename return_type, typename... args>
std::system_error check2(func f, return_type &ret, args... a) noexcept
{
    if ((ret = f(a...)) == -1)
        return std::system_error(errno, std::system_category());
    else
        return std::system_error(0, std::system_category());
}
}
}
#endif

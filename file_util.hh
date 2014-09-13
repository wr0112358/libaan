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

#ifndef _LIBAAN_FILE_UTIL_HH
#define _LIBAAN_FILE_UTIL_HH

#include <cstdint>
#include <dirent.h>
#include <functional>
#include <fstream>
#include <string>
#include <system_error>

namespace libaan {
namespace util {
namespace file
{
size_t get_file_length(std::ifstream &fp);
size_t read_file(const char *file_name, std::string &buff,
               size_t optional_max_buffer_size = 0);
bool write_file(const char *file_name, const std::string &buff);

#ifndef NO_GOOD
class dir {
public:

// Opening files after a readir_r should happen in inode order to avoid cache
// misses:
// http://marc.info/?l=mutt-dev&m=107226330912347&w=2
// e2fsprogs/contrib/spd_readdir.c
// this sounds like scandir(3) does exactly the same

// TODO:
// return all directory entries in inode order:
// bool get_all_entries(const std::string &path, files_data_structure)
//
// return all directory entries sorted alphabetically:
// bool get_all_entries_sorted(const std::string &path, files_data_structure)

/* EXAMPLE:
    auto const f = [](const std::string &base_path, const struct dirent *p) {
        std::cout << "dirent = {"
                  << "\n\t" << std::to_string(p->d_ino)
                  << "\n\t" << std::to_string(p->d_off)
                  << "\n\t" << std::to_string(p->d_reclen)
                  << "\n\t\"" << p->d_name
                  << "\"\n}\n";
    };
    libaan::util::file::dir::readdir("/proc", f);
*/

  static std::error_code
  readdir(const std::string &path,
          const std::function<void(const std::string &, dirent *)> &f);

private:
  static size_t dirent_buf_size(DIR *dirp);
};
#endif

}
}
}

inline size_t libaan::util::file::get_file_length(std::ifstream & fp)
{
    fp.seekg(0, fp.end);
    const auto length = fp.tellg();
    if(length == std::fstream::pos_type(-1))
        return 0;
    fp.seekg(0, fp.beg);
    return length;
}

// TODO: read_file for 0-length files eg procentries:
//template<std::size_t buffer_size> bool read_file(const char *, std::string &)
// TODO: read_file(const std::string &file_name, ...)
// Use optional_max_buffer_size argument to limit the amount of data read. Can
// also be used for "virtual files" with size 0.
// Usage example:
// std::string file_buffer;
// file_buffer.resize(read_file(path + "/proc/cmdline", file_buffer, 512));
inline size_t libaan::util::file::read_file(const char *file_name,
                                            std::string &buff,
                                            size_t optional_max_buffer_size)
{
    std::ifstream fp(file_name);
    const size_t length = optional_max_buffer_size
        ? optional_max_buffer_size
        : get_file_length(fp);
    buff.resize(length);
    char *begin = &*buff.begin();
    fp.read(begin, length);

    return fp.gcount();
}

inline bool libaan::util::file::write_file(const char *file_name,
                                           const std::string &buff)
{
    std::ofstream fp(file_name);

    const char *begin = &*buff.begin();
    fp.write(begin, buff.length());

    return true;
}

#ifndef NO_GOOD
#include <cstddef>
#include <iostream>
#include <memory>

// copied from:
// http://lists.grok.org.uk/pipermail/full-disclosure/2005-November/038295.html
inline size_t libaan::util::file::dir::dirent_buf_size(DIR *dirp)
{
    long name_max;
#if defined(HAVE_FPATHCONF) && defined(HAVE_DIRFD) && defined(_PC_NAME_MAX)
    name_max = fpathconf(dirfd(dirp), _PC_NAME_MAX);
    if(name_max == -1)
    #if defined(NAME_MAX)
        name_max = NAME_MAX;
    #else
        return (size_t)(-1);
    #endif
#else
        static_cast<void>(dirp);
    #if defined(NAME_MAX)
    name_max = NAME_MAX;
    #else
    #error "buffer size for readdir_r cannot be determined"
    #endif
#endif
    return (size_t)offsetof(struct dirent, d_name) + name_max + 1;
}

inline std::error_code libaan::util::file::dir::readdir(
    const std::string &path,
    const std::function<void(const std::string &, dirent *)> &callback)
{
    DIR *dirp = opendir(path.c_str());
    if (!dirp)
        return std::system_error(errno, std::system_category()).code();

    // calculate size for readdir
    size_t size;
    {
        const auto errno_backup = errno;
        size = dirent_buf_size(dirp);
        if (size == static_cast<size_t>(-1)) {
            if(errno_backup != errno)
                return std::system_error(errno, std::system_category()).code();
            // "If the system does not have a limit ... for the resource ..."
            // TODO: other programs mostly "take a guess".. see link for
            // dirent_buf_size(...)
            return std::system_error(EINVAL, std::system_category()).code();
        }
    }

    // allocate buffer to correct size
    // TODO: cache this?
    std::unique_ptr<uint8_t[]> tmp_buf(new uint8_t[size]);
    struct dirent *buf = reinterpret_cast<struct dirent *>(tmp_buf.get());

    struct dirent *entry;
    do {
        const auto error = readdir_r(dirp, buf, &entry);

        // Bad file descriptor
        if(error)
            return std::system_error(errno, std::system_category()).code();

        // end of stream
        if(!entry)
            break;

        // entry valid
        callback(path, entry);
    } while(true);

    // TODO: readdir_r: Operation not permitted
    return std::system_error(0, std::system_category()).code();
}
#endif

#endif


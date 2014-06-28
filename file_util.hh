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

namespace libaan {
namespace util {
namespace file
{
size_t get_file_length(std::ifstream &fp);
bool read_file(const char *file_name, std::string &buff);
bool write_file(const char *file_name, const std::string &buff);

class dir {
public:
  /* EXAMPLE:
  void procfs::parser::parse(procfs & p) const
  {
      auto const f = [](const std::string & base_path, const struct dirent * p)
  {
          std::cout << "struct dirent = { " << std::endl
          << "\t" << std::to_string(p->d_ino) << std::endl
          << "\t" << std::to_string(p->d_off) << std::endl
          << "\t" << std::to_string(p->d_reclen) << std::endl
          << "\t\"" << p->d_name << "\"" << std::endl
          << "}" << std::endl;
      };

      using libaan::util::file;
      dir::readdir("/proc", f);
  }
  */
  static void
  readdir(const std::string &path,
          const std::function<void(const std::string &, dirent *)> &f);

private:
  static size_t dirent_buf_size(DIR *dirp);
};
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

inline bool libaan::util::file::read_file(const char *file_name,
                                          std::string &buff)
{
    std::ifstream fp(file_name);
    const size_t length = get_file_length(fp);
    buff.resize(length);
    char *begin = &*buff.begin();
    fp.read(begin, length);

    return true;
}

inline bool libaan::util::file::write_file(const char *file_name,
                                           const std::string &buff)
{
    std::ofstream fp(file_name);

    const char *begin = &*buff.begin();
    fp.write(begin, buff.length());

    return true;
}

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

// TODO: possible to add template argument to lambda function?
inline void libaan::util::file::dir::readdir(
    const std::string &path,
    const std::function<void(const std::string &, dirent *)> &f)
{
    DIR *dirp = opendir(path.c_str());
    if (!dirp)
        return; // TODO

    size_t size = dirent_buf_size(dirp);
    if (size == static_cast<size_t>(-1)) {
        // TODO
    }

    std::unique_ptr<uint8_t[]> tmp_buf(new uint8_t[size]);
    struct dirent *buf = reinterpret_cast<struct dirent *>(tmp_buf.get());
    if (!buf) {
        // TODO
    }

    int error;
    struct dirent *ent = nullptr;
    do {
        error = readdir_r(dirp, buf, &ent);
        if(!ent)
            break;
        if(error) {
            perror(std::string("readdir_r(" + std::string(ent->d_name)
                               + ") non-fatal error").c_str());
        }
        // std::cout <<  << std::endl;
        f(path, ent);
    } while(true);

    if(!ent && error) {
        perror("readdir_r");
        // TODO: eg
        // readdir_r: Operation not permitted
    }
}

#endif


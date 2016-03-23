#include "file.hh"

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <fstream>
#include <iostream>

#include <ext/stdio_filebuf.h>
#include <fcntl.h>
#include <unistd.h>

#include <libaan/debug.hh>

bool libaan::real_path(const std::string &path, std::string &result)
{
    std::string tmp(PATH_MAX, '\0');
    if(realpath(path.c_str(), &tmp[0]) == NULL)
        return false;
    libaan::strip(tmp);
    std::swap(result, tmp);
    return true;
}

// copied from:
// http://lists.grok.org.uk/pipermail/full-disclosure/2005-November/038295.html
size_t libaan::dirent_buf_size(DIR *dirp)
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

inline size_t libaan::get_file_length(std::ifstream & fp)
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
size_t libaan::read_file(const char *file_name,
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

bool libaan::write_file(const char *file_name,
                                           const std::string &buff)
{
    std::ofstream fp(file_name);

    const char *begin = &*buff.begin();
    fp.write(begin, buff.length());

    return true;
}

bool libaan::readdir(
    const std::string &path,
    const std::function<void(const std::string &, dirent *)> &callback)
{
    DIR *dirp = opendir(path.c_str());
    if (!dirp)
        return false;

    // calculate size for readdir
    size_t size;
    {
        errno = 0;
        size = dirent_buf_size(dirp);
        if (size == static_cast<size_t>(-1)) {
            if(errno == 0)
                errno = EINVAL;
            return false;
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
            return false;

        // end of stream
        if(!entry)
            break;

        // entry valid
        callback(path, entry);
    } while(true);

    // TODO: readdir_r: Operation not permitted
    return true;
}

std::string libaan::temp_file_path()
{
    std::string buf(L_tmpnam, '\0');
    return tmpnam(&buf[0]) ? buf : "";
}

bool libaan::fd_to_stream(std::ofstream &of, int fd)
{
    // TODO: flags must be consistent
    __gnu_cxx::stdio_filebuf<char> fb(fd, std::ios::in | std::ios::out
                                      | std::ios::app);
    of.std::ios::rdbuf(&fb);
    return !of.bad();
}

std::string libaan::path_of_fd(int fd)
{
    const auto path = readlink(std::string("/proc/self/fd/").append(std::to_string(fd)));
    // if(path.empty()) perror("readlink");
    return path;
}

bool libaan::temp_file(std::ofstream &of, const std::string &suffix, const std::string &prefix)
{
    std::string prefix_total(prefix.length() + 6 + suffix.length() + 1, '\0');
    prefix_total.insert(0, prefix);
    prefix_total.insert(prefix.length(), "XXXXXX");
    prefix_total.insert(prefix.length() + 6, suffix);
    int fd = mkostemps(&prefix_total[0], suffix.length(), O_APPEND);
//    std::cout << "a: fd=" << fd << ", path=\"" << path_of_fd(fd) << "\"\n";
    if(fd == -1) {
        of.clear(std::ios_base::badbit);
        return false;
    }
//    std::cout << "b: fd=" << fd << ", path=\"" << path_of_fd(fd) << "\"\n";
    return fd_to_stream(of, fd);
}

std::string libaan::readlink(const std::string &path)
{
    std::string buffer;
    buffer.resize(1024);
    const ssize_t byte_count = ::readlink(path.c_str(), &buffer[0], 1024);
    if(byte_count == -1)
        return std::string();

    // readlink(3) not necessarily returns a null-terminated string. Use
    // byte_count to determine length.
    buffer.resize(byte_count);
    return buffer;
}

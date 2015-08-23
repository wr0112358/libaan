#ifndef _LIBAAN_FILE_HH_
#define _LIBAAN_FILE_HH_

#include <dirent.h>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

namespace libaan {

size_t get_file_length(std::ifstream &fp);
size_t read_file(const char *file_name, std::string &buff,
               size_t optional_max_buffer_size = 0);
bool write_file(const char *file_name, const std::string &buff);

size_t dirent_buf_size(DIR *dirp);

// TODO: better use template + lambda
// TODO: if lambda returns false, interrupt read
bool readdir(const std::string &path,
             const std::function<void(const std::string &, dirent *)> &f);

template<typename lambda_t>
bool readdir2(const std::string &path,
             //bool(const std::string &, dirent *)
             lambda_t lambda);

std::string temp_file_path();

// expects fd to be opened with O_RDWR and O_APPEND, or with fopen(.., "rw+")
bool fd_to_stream(std::ofstream &of, int fd);
std::string path_of_fd(int fd);

bool temp_file(std::ofstream &of, const std::string &suffix = "", const std::string &prefix = "/tmp/");

// it is assumed, we have access permissions to path.
std::string readlink(const std::string &path);

}

template<typename lambda_t>
bool libaan::readdir2(const std::string &path,
                      //bool(const std::string &, dirent *)
                      lambda_t lambda)
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
        if(!lambda(path, entry))
            break;
    } while(true);

    return true;
}

#endif

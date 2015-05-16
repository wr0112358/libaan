#ifndef _LIBAAN_FILE_HH_
#define _LIBAAN_FILE_HH_

#include <dirent.h>
#include <fstream>
#include <functional>
#include <string>

namespace libaan {

size_t get_file_length(std::ifstream &fp);
size_t read_file(const char *file_name, std::string &buff,
               size_t optional_max_buffer_size = 0);
bool write_file(const char *file_name, const std::string &buff);

// TODO: better use template + lambda
bool readdir(const std::string &path,
             const std::function<void(const std::string &, dirent *)> &f);

std::string temp_file_path();

// expects fd to be opened with O_RDWR and O_APPEND, or with fopen(.., "rw+")
bool fd_to_stream(std::ofstream &of, int fd);
std::string path_of_fd(int fd);

bool temp_file(std::ofstream &of, const std::string &suffix = "", const std::string &prefix = "/tmp/");

// it is assumed, we have access permissions to path.
std::string readlink(const std::string &path);

}

#endif

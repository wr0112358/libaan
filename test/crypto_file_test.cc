#include "libaan/crypto_file.hh"
#include "libaan/file.hh"

#include <gtest/gtest.h>

/*

#include <cstdlib>
#include <unistd.h>

// it is assumed, we have access permissions to path.
inline std::string readlink(const std::string &path)
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

std::string temp_file(std::string &&prefix, std::string &&suffix)
{
    int fd = mkstemps(prefix.append("XXXXXX").append(suffix).c_str(), suffix.length());
    if(fd == -1) {
        perror("mkstemps");
        return "";
    }

    const auto path = readlink(std::string("/proc/self/fd/").append(std::to_string(fd)));
    if(path.empty()) {
        perror("readlink");
        close(fd);
        // unlink ??
        return "";
    }
    return path;
}
*/

TEST(crypto_file_hh, a) {
    //const auto path = temp_file("crypto_file_test", "");
    const auto path = libaan::temp_file_path();
    EXPECT_FALSE(path.empty());
    std::cout << path << "\n";
    libaan::crypto_file crypt(path);
    
}

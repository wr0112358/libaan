
#include "libaan/crypto.hh"
#include "libaan/crypto_file.hh"
#include "libaan/file.hh"
#include <unistd.h>

#include <iostream>

namespace {

void print_usage(int, char *argv[])
{
    std::cout << "USAGE:\n" << std::string(argv[0])
              << " file_to_convert\n";
}

}

// create ciphertext in programm, then decrypt again works.
// rerad in encrypted file fails
bool convert(const std::string &file)
{
    const libaan::password_from_stdin pw2(6);
    if(!pw2.have_password)
        return false;

    using namespace libaan;
    crypto_file file_io(file);

    if(!file_io.read_and_parse_old_version_0010(pw2.password)) {
        std::cout << "crypto_file::read_and_parse_old_version_0010() failed\n";
        return false;
    }

    crypto_file::error_type err = file_io.write(pw2.password);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::write failed: "
                  << crypto_file::error_string(err) << "\n";
        exit(EXIT_FAILURE);
    }

    return true;
}

int main(int argc, char *argv[])
{
    print_usage(argc, argv);
    if(argc != 2) {
        exit(EXIT_FAILURE);
    }

    const std::string pw(argv[1]);
    convert(pw); 

    exit(EXIT_SUCCESS);
}

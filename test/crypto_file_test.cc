
#include "../crypto_camellia.hh"
#include "../crypto_file.hh"
#include "../crypto_util.hh"
#include "../file_util.hh"
#include <unistd.h>

namespace {
const auto PLAIN = "crypto_test_input";
const auto CIPHER = "crypto_test_input.crypt";

void print_usage(int, char *argv[])
{
    std::cout << "USAGE:\n" << std::string(argv[0])
              << " password\n";
}

}

// create ciphertext in programm, then decrypt again works.
// rerad in encrypted file fails
bool test1(const std::string &pw)
{
/* Works
    const password_from_stdin pw2(6);
    std::cout << "password_from_stdin: ";
    if(pw2.have_password)
        std:: cout << "\"" << pw2.password << "\"\n";
    else {
        std::cout << "failed.\n";
        return false;
    }
*/
    std::string plain_buff;
    if(!libaan::util::file::read_file(PLAIN, plain_buff)) {
        std::cout << "read_file failed\n";
        return false;
    }
    using namespace libaan::crypto::file;
    crypto_file file_io(CIPHER);
    crypto_file::error_type err = crypto_file::NO_ERROR;
    err = file_io.read(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::read() failed: "
                  << crypto_file::error_string(err) << "\n";
        return false;
    }

    // set content
    file_io.get_decrypted_buffer().assign(plain_buff);

    err = file_io.write(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::write failed: "
                  << crypto_file::error_string(err) << "\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "encryption and write successfull.\nReading encrypted file again..\n";

    crypto_file file_io2(CIPHER);
    err = file_io2.read(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::read() failed: "
                  << crypto_file::error_string(err) << "\n";
        return false;
    }
    if(file_io2.get_decrypted_buffer() != plain_buff) {
        std::cout << "Error: plain != Decrypt(Encrypt(plain))\n";
        return false;
    }
    std::cout << "Success: plain == Decrypt(Encrypt(plain))\n";

    return true;
}

int main(int argc, char *argv[])
{
    print_usage(argc, argv);
    if(argc != 2) {
        exit(EXIT_FAILURE);
    }

    const std::string pw(argv[1]);
    test1(pw); 

    exit(EXIT_SUCCESS);
}

// test/crypto_util_test_lion password test/test_file test/test_file.crypt
#include "libaan/crypto_pbkdf2_pkcs5.hh"
#include "libaan/crypto_untested.hh"
#include "libaan/file_util.hh"

#include <iomanip>
#include <iostream>

inline void hexify(const std::string & s)
{
    for(char c: s)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)((unsigned char)c) << " ";
}

void debug_print_key(const std::string & pw, const std::string & key)
{
    std::cout << "password: " << pw << "\n"
              << "hex(password): ";
    hexify(pw);
    std::cout << "\n"
              << "hex(key): ";
    hexify(key);
    std::cout << "\n";
}

bool get_key(const std::string & pw, std::string & key)
{
    const size_t iteration_count = 1000;
    std::string salt(16, 0);

/*
    if(!read_random_bytes(16, salt)) {// 128 bit salt
        std::cout << "read from /dev/random failed\n";
        return false;
    }
*/

    using namespace libaan::crypto;
    if(!pbkdf2_pkcs5::pbkdf2(pw, salt, iteration_count, key)) {
        std::cout << "pbkdf2 key generation failed.\n";
        return false;
    }

    return true; 
}

void print_usage(int argc, char *argv[])
{
    std::cout << "USAGE:\n" << std::string(argv[0]) << " password file_to_encrypt destination_file\n";
}

int main(int argc, char *argv[])
{
    print_usage(argc, argv);
    if(argc != 4)
        exit(EXIT_FAILURE);

    const std::string src(argv[2]);
    const std::string dst(argv[3]);
    const std::string pw(argv[1]);
    const size_t key_length = 12;//128;
    std::string key;
    key.resize(key_length);
    get_key(pw, key);

    debug_print_key(pw, key);

    std::string src_buff;
    libaan::util::file::read_file(src.c_str(), src_buff);

    std::cout << "Input File:\n####\n" << src_buff << "####\n";

    using namespace libaan::crypto;
    if(!lion::check_file_size(src_buff.length()))
        std::cout << "lion::check_file_size(src_file) failed\n";
    std::string dst_buff;
    dst_buff.resize(src_buff.length());
/* TODO:
- segfaults
  -> only for very small inputs
  -> int tmp[HASH_WORDS] in encrypt() expects 32bit type?
- blocksize == filesize
*/
    lion::encrypt(reinterpret_cast<const unsigned char *>(src_buff.c_str()),
                  reinterpret_cast<unsigned char *>(&dst_buff[0]),
                  src_buff.length(), reinterpret_cast<const unsigned char *>(&key[0]));

    std::cout << "\n"
              << "hex(lion(src_buff)):\n####\n";
    hexify(dst_buff);
    std::cout << "\n####\n";

    libaan::util::file::write_file(dst.c_str(), dst_buff);

    //const std::string cipher_text(dst_buff);
    std::string cipher_text;
    libaan::util::file::read_file(dst.c_str(), cipher_text);
    std::string dst_decrypted;
    dst_decrypted.resize(cipher_text.length());
    lion::decrypt(reinterpret_cast<const unsigned char *>(cipher_text.c_str()),
                  reinterpret_cast<unsigned char *>(&dst_decrypted[0]),
                  cipher_text.length(), reinterpret_cast<const unsigned char *>(&key[0]));

    std::cout << "\n"
              << "lion_decrypt(lion_encrypt(src_buff))):\n####\n"
              << dst_decrypted << "\n####\n";

    if(dst_decrypted != src_buff)
        std::cout << "Error: Decrypted text not equal input text.\n";
    else
        std::cout << "Ok: Decrypted text is equal input text.\n";
// TODO test this API
    //file_encryption::crypto_file cf(argv[1]);
    //auto err = cf.read();
 
    return 0;
}

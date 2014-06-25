#include "libaan/crypto_pbkdf2_pkcs5.hh"
#include "libaan/file_util.hh"

#include <iomanip>
#include <iostream>

bool read_random_bytes(size_t count, std::string & bytes)
{
	std::ifstream f("/dev/random");
	f.read(&bytes[0], count);
	return true;
}

void hex(const std::string & s)
{
    for(char c: s)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)((unsigned char)c) << " ";
}

void test_pbkdf2_print(const std::string & pw, const std::string & key, const std::string & salt)
{
    std::cout << "password: " << pw << "\n"
              << "hex(password): ";
    hex(pw);
    std::cout << "\n"
              << "hex(salt): ";
    hex(salt);
    std::cout << "\n"
              << "hex(key): ";
    hex(key);
    std::cout << "\n";
}

void test_pbkdf2()
{
    const std::string pw = "my_password_1";
    const size_t iteration_count = 1000;
    std::string salt(16, 0);
    const size_t key_length = 12;//128;
    std::string key;
    key.resize(key_length);

    using namespace libaan::crypto;
    if(!pbkdf2_pkcs5::pbkdf2(pw, salt, iteration_count, key))
        std::cout << "read_random_bytes failed\n";
    test_pbkdf2_print(pw, key, salt);

    if(!read_random_bytes(16, salt)) // 128 bit salt
        std::cout << "read_random_bytes failed\n";
    if(!pbkdf2_pkcs5::pbkdf2(pw, salt, iteration_count, key))
        std::cout << "read_random_bytes failed\n";

    test_pbkdf2_print(pw, key, salt);

}

int main()
{
    //file_encryption::crypto_file cf(argv[1]);
    //auto err = cf.read();
// TODO

    //std::string buff;
    //read_file(argv[1], buff);

    test_pbkdf2();
 
    return 0;
}

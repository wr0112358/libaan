#include "../crypto_camellia.hh"
#include "../crypto_pbkdf2_pkcs5.hh"
#include "../file_util.hh"

#include <iomanip>
#include <iostream>

std::string create_pw(const std::string &ascii_set, std::size_t count)
{
    std::string pw;
    pw.resize(count);
    if(!libaan::crypto::read_random_ascii_set(count, ascii_set, pw)) {
        std::cerr << "Error creating a password from random data. Aborting.\n";
        return "error";
    }
    return pw;
}

void create_pws()
{
    std::cout << "Creating test pws with pseudo rng data:\n";
    std::string set;
    for(int i = 0; i < 255; i++)
        if(isprint(i))
            set.push_back(char(i));

    for(int j = 6; j < 13; j++) {
        std::cout << "length = " << j << "\n";
        for(int i = 0; i < 4; i++)
            std::cout << create_pw(set, j) << "\n";
        std::cout << "\n";
    }
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

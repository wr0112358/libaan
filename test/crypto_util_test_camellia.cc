//rm test_file.crypt test_file6decrypted; ./crypto_util_test_camellia password test_file6 test_file.crypt

#include "libaan/crypto_camellia.hh"
#include "libaan/file_util.hh"

/*
static void line_wise_diff(const std::string & l, const std::string & r, std::string & result)
{
    result = "";
    std::istringstream lss(l);
    std::istringstream rss(r);
    std::string lline, rline;
    size_t line_nr = 0;
    while(true) {
        const bool lok = std::getline(lss, lline);
        const bool rok = std::getline(rss, rline);
        if(!lok && !rok)
            break;
        if(!lok)
            result += std::to_string(line_nr) + ": > " + rline + "\n";
        else if(!rok)
            result += std::to_string(line_nr) + ": < " + lline + "\n";
        else if(lline != rline){
            result = result + std::to_string(line_nr) + ": " + lline + "\n";
            result += "!=\n";
            result = result + std::to_string(line_nr) + ": " + rline + "\n";
        }
        line_nr++;
    }
}
*/

void debug_print_key(const std::string & pw, const std::string & key)
{
    std::cout << "password: " << pw << "\n"
              << "hex(password): ";
    hex(pw);
    std::cout << "\n"
              << "hex(key): ";
    hex(key);
    std::cout << "\n";
}

void print_usage(const std::string & argv0)
{
    std::cout << "USAGE:\n" << argv0 << " password file_to_encrypt destination_file\n";
}

int main(int argc, char *argv[])
{
    print_usage(argv[0]);
    if(argc != 4)
        exit(EXIT_FAILURE);

    const std::string src(argv[2]);
    const std::string dst(argv[3]);
    const std::string pw(argv[1]);

    std::string src_buff;
    libaan::util::file::read_file(src.c_str(), src_buff);

    std::cout << "Input File:\n####\n" << src_buff << "####\n";

    using namespace libaan::crypto;

    std::string dst_buff;
    dst_buff.resize(src_buff.length());

    libaan::crypto::camellia::camellia_256 cipher;
    if(!cipher.init()) {
        std::cout << "camellia_256::init() failed\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "camellia_256::init() successfull\n";

    if(!cipher.encrypt(pw, src_buff, dst_buff)) {
        std::cout << "camellia_256::encrypt() failed\n";
        exit(EXIT_FAILURE);
    }

/*
    std::cout << "hex(encrypt(src)):" << "\n";
    hex(dst_buff);
    std::cout << "\n";
*/
    libaan::util::file::write_file(dst.c_str(), dst_buff);

    //const std::string cipher_text(dst_buff);
    std::string cipher_text;
    libaan::util::file::read_file(dst.c_str(), cipher_text);
    std::string dst_decrypted;
    dst_decrypted.resize(cipher_text.length());

    if(!cipher.decrypt(pw, cipher_text, dst_decrypted)) {
        std::cout << "camellia_256::decrypt() failed\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "\n"
              << "camellia_256::decrypt(camellia_256::encrypt(src_buff))):\n####\n"
              << dst_decrypted << "\n####\n";

    if(dst_decrypted != src_buff) {
        std::cout << "Error: Decrypted text not equal input text:\n";
/*
        std::string diff;
        line_wise_diff(src_buff, dst_decrypted, diff);
        std::cout << diff;
*/
    } else
        std::cout << "Ok: Decrypted text is equal input text.\n";

    libaan::util::file::write_file(std::string(src + std::string("decrypted")).c_str(), dst_decrypted);

// TODO test this API
    //file_encryption::crypto_file cf(argv[1]);
    //auto err = cf.read();
 
    return 0;
}

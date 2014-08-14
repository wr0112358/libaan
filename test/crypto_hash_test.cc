#include "../crypto_hash.hh"

#include <iomanip>
#include <iostream>

void hex(const std::string & s)
{
    for(char c: s)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)((unsigned char)c) << " ";
}

void print(const std::string &prefix, const std::string &to_hex)
{
    std::cout << prefix << ":\n";
    hex(to_hex);
    std::cout << "\n";
}

void hmac(const std::string &in, const std::string &key)
{
    /*
      Example result:
      in = "hello world", key = 012345678
      hmac = e1 9e 22 01 22 b3 7b 70 8b fb 95 ac a2 57 79 05 ac ab f0 c0
     */
    libaan::crypto::hash h;
    std::string out;
    h.sha1_hmac(in, key, out);
    print("hmac:", out);
}

void sha(const std::string &in)
{
    libaan::crypto::hash h;
    std::string out;
    h.sha1(in, out);
    print("sha1:", out);
}

int main(int argc, char *argv[])
{
    if(argc == 3)
        hmac(argv[1], argv[2]);
    else if(argc == 2)
        sha(argv[1]);
    else {
        std::cout << "Usage:\nto generate a hmac run: " << argv[0] << " <input_text> <key>\n"
                  << "to generate a sha1 run: " << argv[0] << "<input_text>\n";
        return -1;
    }
    
    return 0;
}

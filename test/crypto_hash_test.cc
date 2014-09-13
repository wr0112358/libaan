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
    std::cout << std::dec << "\n";
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

void hmac_inc(const std::string &in, const std::string &key)
{
    if(in.size() % 4) {
        std::cout << "size needs to be multiple of 4\n";
        return;
    }

    std::string out;
    {
        libaan::crypto::hmac_incremental h(key, out);
        const auto f = in.size() / 4;
        std::cout << "h.update 1: 0 -> " << f << "\n";
        h.update(in.substr(0, f));
        std::cout << "h.update 2: " << f << " -> " << 2*f << "\n";
        h.update(in.substr(f, f));
        std::cout << "h.update 3: " << 2*f << " -> " << 3*f << "\n";
        h.update(in.substr(f + f, f));
        std::cout << "h.update 4: " << 3*f << " -> " << 4*f << "\n";
        h.update(in.substr(f + f + f, f));
    }
    print("hmac_incremental:", out);
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
    if(argc == 3) {
        hmac(argv[1], argv[2]);
        hmac_inc(argv[1], argv[2]);
    } else if(argc == 2)
        sha(argv[1]);
    else {
        std::cout << "Usage:\nto generate a hmac run: " << argv[0] << " <input_text> <key>\n"
                  << "to generate a sha1 run: " << argv[0] << "<input_text>\n";
        return -1;
    }
    
    return 0;
}

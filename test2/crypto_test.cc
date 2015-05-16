#include "libaan/crypto.hh"
#include "libaan/algorithm.hh"
#include "libaan/debug.hh"
#include "libaan/string.hh"

#include <gtest/gtest.h>

TEST(crypto_hh, read_random_bytes_noblock) {
    for(auto count: { 0u, 1u, 10u, 4096u }) {
        std::string b1;
        EXPECT_TRUE(libaan::read_random_bytes_noblock(count, b1));
        EXPECT_EQ(b1.size(), count);

        if(b1.size() > 1)
            EXPECT_FALSE(libaan::all_eq(b1));
    }
}

namespace {
std::vector<unsigned char> convert(const std::string &s)
{
    std::vector<unsigned char> v;
    v.reserve(s.size());
    for(const auto c: s)
        v.push_back((unsigned char)c);
    return v;
}

template<typename c_t>
void print(const c_t &s, size_t max = 240)
{
    size_t i = 0;
    for(auto c: s) {
        if(i >= max) { break; }
        std::cout << std::hex << (int)c << " ";
        if(++i % 40 == 0)
            std::cout << "\n";
    }
    std::cout << std::dec << "\n";
}
}

TEST(crypto_hh, read_random_ascii_set) {
    std::string s;
    s.reserve(std::numeric_limits<char>::max());
    for(char c = 0; c < std::numeric_limits<char>::max(); c++)
        s[c] = c;

    for(auto count: { 0u, 1u, 10u, 4096u }) {
        for(std::string set: { std::string(""), std::string("a"),
                    std::string("abc"), std::string("abcdef0123456789"), s }) {
            std::string b;
            EXPECT_TRUE(libaan::read_random_ascii_set(count, set, b));
            if(set.empty())
                EXPECT_EQ(b.size(), 0);
            else
                EXPECT_EQ(b.size(), count);

            if(b.size() > 1) {
                EXPECT_TRUE(libaan::contains_only(b, set));
            }
        }
    }
}

TEST(crypto_hh, hmac) {
    std::string s;
    s.reserve(std::numeric_limits<char>::max());
    for(char c = 0; c < std::numeric_limits<char>::max(); c++)
        s[c] = c;

    for(auto count: { 0u, 1u, 10u, 769u, 4096u }) {
        for(std::string key: { std::string(""), std::string("a"),
                    std::string("abc"), std::string("abcdef0123456789"), s }) {

            std::string hmac;
            std::string input;
            {
                libaan::hmac h(key, hmac);
                EXPECT_TRUE(h.state);

                EXPECT_TRUE(libaan::read_random_bytes_noblock(count, input));
                EXPECT_TRUE(h.update(input));
                EXPECT_TRUE(h.state);
            }

            EXPECT_EQ(hmac.size(), libaan::hmac::SIZE);
            EXPECT_FALSE(libaan::all_eq(hmac));

            if(libaan::all_eq(hmac)) {
                std::cout << "HMAC FAILED for input(count= " << count << ", key = \"" << key << "\"):\n";
                print(input);
                std::cout << "\nresult:\n";
                print(hmac);
                std::cout << "\n\n";
            }
        }
    }

    /* wikipedia exmaples
      HMAC_SHA1("key", "The quick brown fox jumps over the lazy dog")   = 0xde7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9
      HMAC_SHA1("", "")   = 0xfbdb1d1b18aa6c08324b7d64b71fb76370690e1d
    */

    {
        std::string hmac;
        {
            libaan::hmac h("key", hmac);
            h.update("The quick brown fox jumps over the lazy dog");
        }
        EXPECT_EQ(convert(hmac), libaan::hex2bin("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9", 20).second);
    }

/* Does not work: null key results in openssl reusing an existing key. Behavior for empty key is undocumented.
    {
        std::string hmac;
        {
            libaan::hmac h("", hmac);
            h.update("The quick brown fox jumps over the lazy dog");
        }

        const auto b = libaan::hex2bin("fbdb1d1b18aa6c08324b7d64b71fb76370690e1d", 20);
 
        std::cout << "i: " << libaan::bin2hex(b.second) << "\n";
        std::cout << "i2: " << libaan::bin2hex(convert(hmac)) << "\n";
        std::cout << "a: "; print(convert(hmac));
        std::cout << "\nb: (" << std::boolalpha  << b.first << std::noboolalpha << ", ";
        print(b.second); std::cout << ")\n";
        EXPECT_EQ(convert(hmac), libaan::hex2bin("fbdb1d1b18aa6c08324b7d64b71fb76370690e1d", 20).second);
    }
*/
}

TEST(crypto_hh, hash) {
    // TODO
}

TEST(crypto_hh, pkcs5) {
    // TODO
}

TEST(crypto_hh, pbkdf2) {
    // TODO
}

TEST(crypto_hh, camellia_256) {
    // TODO
}

TEST(crypto_hh, lion) {
    // TODO
}

TEST(crypto_hh, ranf) {
    // TODO
}

#include "libaan/debug.hh"

#include <gtest/gtest.h>

#include <cstring>

void t(const char *a)
{
    const auto l = std::strlen(a);
    const auto r = libaan::hex2bin(a, l);
    EXPECT_TRUE(r.first);
    EXPECT_EQ(l / 2, r.second.size());

/*
    // +1 so snprintf can write its zero byte at the end
    std::string s(l + 1, '\0');

    for(size_t i = 0; i < r.second.size(); i++) {
        printf("%02x ", (int)r.second[i]);
        snprintf(&s[i * 2], 3, "%02x", (int)r.second[i]);
        std::cout << s << "\n";
    }
    puts("");
*/

    auto s = libaan::bin2hex(r.second);

    libaan::strip(s);
     std::cout << s << "\n" << a << "\n" << std::string(a) << "\nl = " << s.length() << "/" << std::string(a).length()<< "/" << std::strlen(a) << "\n";
    EXPECT_TRUE(std::strcmp(s.c_str(), a) == 0);
    EXPECT_TRUE(s.compare(a) == 0);
    EXPECT_TRUE(s == a);
}

TEST(debug_hh, hex2bin) {
    t("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9");
    t("fbdb1d1b18aa6c08324b7d64b71fb76370690e1d");
}

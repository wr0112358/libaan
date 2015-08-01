#include "libaan/algorithm.hh"
//#include "libaan/debug.hh"
//#include "libaan/string.hh"

#include <gtest/gtest.h>

TEST(algorithm_hh, bin_remove_leading_0) {
    std::vector<std::tuple<std::string, std::string, std::string> > ts {
        std::make_tuple("000abc000", "abc000", "000abc000"),
            std::make_tuple("abc000", "abc000", "abc000"),
            std::make_tuple("00000000abc000", "abc000", "abc000"),
            std::make_tuple("000000000000000abc000", "abc000", "0000000abc000"),
            std::make_tuple("0000000000000000abc000", "abc000", "abc000"),
            std::make_tuple("00000000000000000abc000", "abc000", "0abc000") };

    for(const auto &t: ts) {
        auto i1 { std::get<0>(t) };
        auto i2 { std::get<0>(t) };
        libaan::bin_remove_leading_0(i1);
        libaan::bin_remove_leading_0_full_byte(i2);
        EXPECT_EQ(i1, std::get<1>(t));
        EXPECT_EQ(i2, std::get<2>(t));
    }
}

TEST(algorithm_hh, contains_only) {
    const std::string s1{ "abcde" };
    EXPECT_TRUE(libaan::contains_only(s1, s1));
    const std::string s2{ "aaaa" };
    EXPECT_FALSE(libaan::contains_only(s1, s2));
    EXPECT_TRUE(libaan::contains_only(s2, s1));

    const std::string se { "" };
    EXPECT_TRUE(libaan::contains_only(se, se));
    EXPECT_TRUE(libaan::contains_only(se, s1));
    EXPECT_TRUE(libaan::contains_only(se, s2));
    EXPECT_FALSE(libaan::contains_only(s1, se));
    EXPECT_FALSE(libaan::contains_only(s2, se));

    const std::vector<unsigned> c { 0u, 1u, 10u, 4096u };
    EXPECT_TRUE(libaan::contains_only(c, c));
    EXPECT_TRUE(libaan::contains_only( { 0 } , c));
    EXPECT_FALSE(libaan::contains_only(c, { 0 }));
    const std::set<unsigned> c2 { 13u, 13u, 13u, 13u };
    EXPECT_TRUE(libaan::contains_only(c2, c2));
    EXPECT_TRUE(libaan::contains_only( { 13u } , c2));
    EXPECT_TRUE(libaan::contains_only(c2, { 13u }));

    EXPECT_TRUE(libaan::contains_only(std::string("\0\0"), { '\0' } ));
    std::string s;
    for(int i = 0; i < 'D'; i++)
        s.push_back((char)i);
    EXPECT_TRUE(libaan::contains_only("ABC", s));
    EXPECT_TRUE(libaan::contains_only("\0 ABC", s));
}

TEST(algorithm_hh, all_eq) {
    const std::vector<unsigned> c { 0u, 1u, 10u, 4096u };
    const std::set<unsigned> c2 { 13u, 13u, 13u, 13u };
    std::string s1{ "abcde" };
    std::string s2{ "aaaaaa" };
    std::string s3{ "acccca" };
    std::string s4{ "axyccde" };
    EXPECT_FALSE(libaan::all_eq(c));
    EXPECT_FALSE(libaan::all_eq2(c));

    EXPECT_TRUE(libaan::all_eq(c2));
    EXPECT_TRUE(libaan::all_eq2(c2));

    EXPECT_FALSE(libaan::all_eq(s1));
    EXPECT_FALSE(libaan::all_eq2(s1));

    EXPECT_TRUE(libaan::all_eq(s2));
    EXPECT_TRUE(libaan::all_eq2(s2));

    EXPECT_FALSE(libaan::all_eq(s3));
    EXPECT_FALSE(libaan::all_eq2(s3));
    EXPECT_FALSE(libaan::all_eq(s4));
    EXPECT_FALSE(libaan::all_eq2(s4));
}

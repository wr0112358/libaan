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
        std::string i1 { std::get<0>(t) };
        std::string i2 { std::get<0>(t) };
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

TEST(algorithm_hh, prefix_suffix_search) {
#define H1 "aabbb"
#define H2 "aacccb"
#define N1 "bb"
#define N2 "cc"
    EXPECT_EQ(libaan::ends_with_prefix(H1, sizeof(H1) - 1, N1, sizeof(N1) - 1), 0);
    EXPECT_EQ(libaan::ends_with_prefix(H1, strlen(H1), N1, strlen(N1)), 0);
    EXPECT_EQ(libaan::ends_with_prefix(H2, sizeof(H2) - 1, N1, sizeof(N1) - 1), 1);
    EXPECT_EQ(libaan::ends_with_prefix(H2, sizeof(H2) - 1, N2, sizeof(N2) - 1), sizeof(N2) - 1);
#undef H1
#undef H2
#undef N1
#undef N2

#define H3 "aabbccdd0000"
#define N3 "000001"
    EXPECT_TRUE(libaan::ends_with(H3, sizeof(H3) - 1, "0000", strlen("0000")));
    EXPECT_TRUE(libaan::ends_with_prefix(H3, sizeof(H3) - 1, N3, sizeof(N3) - 1) == 2);
#undef H3
#undef N3

#define H4 "aabbccdd000"
#define N4 "000001"
    EXPECT_EQ(libaan::ends_with_prefix(H4, sizeof(H4) - 1, N4, sizeof(N4) - 1), 3);
    EXPECT_TRUE(libaan::starts_with(H4, sizeof(H4) - 1, "aa", 2));
    EXPECT_FALSE(libaan::starts_with(H4, sizeof(H4) - 1, "aaa", 3));
    EXPECT_TRUE(libaan::starts_with(H4, sizeof(H4) - 1, "aaa", 2));
#undef H4
#undef N4

#define H5 "0001abcdef"
#define N5 "000001"
    EXPECT_EQ(libaan::starts_with_suffix(H5, sizeof(H5) - 1, N5, sizeof(N5) - 1), 2);
#undef H5
#undef N5

#define H6A "abdefg00"
#define H6B "0001abcdef"
#define N6 "000001"
    EXPECT_EQ(libaan::ends_with_prefix(H6A, sizeof(H6A) - 1, N6, sizeof(N6) - 1), 4);
    EXPECT_EQ(libaan::starts_with_suffix(H6B, sizeof(H6B) - 1, N6, sizeof(N6) - 1), 2);
#undef H6A
#undef H6B
#undef N6

#define H7A "abdefg000"
#define H7B "001abcdef"
#define N7 "000001"
    EXPECT_EQ(libaan::ends_with_prefix(H7A, sizeof(H7A) - 1, N7, sizeof(N7) - 1), 3);
    EXPECT_EQ(libaan::starts_with_suffix(H7B, sizeof(H7B) - 1, N7, sizeof(N7) - 1), 3);
#undef H7A
#undef H7B
#undef N7

#define H8A "abdefg0000"
#define H8B "001abcdef"
#define N8 "000001"
    EXPECT_EQ(libaan::ends_with_prefix(H8A, sizeof(H8A) - 1, N8, sizeof(N8) - 1), 2);
    EXPECT_EQ(libaan::starts_with_suffix(H8B, sizeof(H8B) - 1, N8, sizeof(N8) - 1), 3);
#undef H8A
#undef H8B
#undef N8
}

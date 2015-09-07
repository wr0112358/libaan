#include <iostream>
#include "libaan/byte.hh"

#include <gtest/gtest.h>

TEST(byte_hh, roundtonext8) {
    std::vector<size_t> ts { 0, 1, 7, 8, 15, 16, 17, size_t((2ul<<31) - 1), (2ul<<31), (2ul<<31) + 1 };
    std::vector<size_t> te { 0, 8, 8, 8, 16, 16, 24, 2ul<<31, 2ul<<31, (2ul<<31) + 8 };
    for(size_t i = 0; i < ts.size(); i++)
        EXPECT_EQ(te[i], libaan::roundtonext8(ts[i]));
}

TEST(byte_hh, roundtolast8) {
    std::vector<size_t> ts { 0, 1, 7, 8, 15, 16, 17, size_t((2ul<<31) - 1), (2ul<<31), (2ul<<31) + 1 };
    std::vector<size_t> te { 0, 0, 0, 8, 8, 16, 16, size_t((2ul<<31) - 8), 2ul<<31, 2ul<<31 };
    for(size_t i = 0; i < ts.size(); i++)
        EXPECT_EQ(te[i], libaan::roundtolast8(ts[i]));
}

TEST(byte_hh, count_leading_0) {
    EXPECT_EQ(28, libaan::count_leading_0(uint32_t(0xfULL)));
    EXPECT_EQ(16, libaan::count_leading_0(uint32_t(0xffffULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint32_t(0xffffffffULL)));
    EXPECT_EQ(4, libaan::count_leading_0(uint32_t(0xfffffffULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint32_t(0xf0000000ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint32_t(0xffff0000ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint32_t(0xfffffff0ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint32_t(0xffffffffULL)));

    EXPECT_EQ(60, libaan::count_leading_0(uint64_t(0xfULL)));
    EXPECT_EQ(48, libaan::count_leading_0(uint64_t(0xffffULL)));
    EXPECT_EQ(32, libaan::count_leading_0(uint64_t(0xffffffffULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint64_t(0xffffffffffffffffULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint64_t(0xf000000000000000ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint64_t(0xffff000000000000ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint64_t(0xffffffff00000000ULL)));
    EXPECT_EQ(0, libaan::count_leading_0(uint64_t(0xffffffffffffffffULL)));
}

TEST(byte_hh, count_trailing_0) {
    EXPECT_EQ(0, libaan::count_trailing_0(uint32_t(0xf)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint32_t(0xffff)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint32_t(0xffffffff)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint32_t(0xfffffff)));
    EXPECT_EQ(4, libaan::count_trailing_0(uint32_t(0xfffffff0)));
    EXPECT_EQ(28, libaan::count_trailing_0(uint32_t(0xf0000000)));
    EXPECT_EQ(16, libaan::count_trailing_0(uint32_t(0xffff0000)));
    EXPECT_EQ(4, libaan::count_trailing_0(uint32_t(0xfffffff0)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint32_t(0xffffffff)));

    EXPECT_EQ(0, libaan::count_trailing_0(uint64_t(0xfULL)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint64_t(0xffffULL)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint64_t(0xffffffffULL)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint64_t(0xffffffffffffffffULL)));
    EXPECT_EQ(60, libaan::count_trailing_0(uint64_t(0xf000000000000000ULL)));
    EXPECT_EQ(48, libaan::count_trailing_0(uint64_t(0xffff000000000000ULL)));
    EXPECT_EQ(32, libaan::count_trailing_0(uint64_t(0xffffffff00000000ULL)));
    EXPECT_EQ(0, libaan::count_trailing_0(uint64_t(0xffffffffffffffffULL)));
}


TEST(byte_hh, count_leading_1) {
    EXPECT_EQ(0, libaan::count_leading_1(uint32_t(0xf)));
    EXPECT_EQ(0, libaan::count_leading_1(uint32_t(0xffff)));
    EXPECT_EQ(32, libaan::count_leading_1(uint32_t(0xffffffff)));
    EXPECT_EQ(0, libaan::count_leading_1(uint32_t(0xfffffff)));
    EXPECT_EQ(28, libaan::count_leading_1(uint32_t(0xfffffff0)));
    EXPECT_EQ(4, libaan::count_leading_1(uint32_t(0xf0000000)));
    EXPECT_EQ(16, libaan::count_leading_1(uint32_t(0xffff0000)));
    EXPECT_EQ(28, libaan::count_leading_1(uint32_t(0xfffffff0)));
    EXPECT_EQ(12, libaan::count_leading_1(uint32_t(0xfff00fff)));

    EXPECT_EQ(0, libaan::count_leading_1(uint64_t(0xfULL)));
    EXPECT_EQ(0, libaan::count_leading_1(uint64_t(0xffffULL)));
    EXPECT_EQ(0, libaan::count_leading_1(uint64_t(0xffffffffULL)));
    EXPECT_EQ(64, libaan::count_leading_1(uint64_t(0xffffffffffffffffULL)));
    EXPECT_EQ(4, libaan::count_leading_1(uint64_t(0xf000000000000000ULL)));
    EXPECT_EQ(16, libaan::count_leading_1(uint64_t(0xffff000000000000ULL)));
    EXPECT_EQ(32, libaan::count_leading_1(uint64_t(0xffffffff00000000ULL)));
    EXPECT_EQ(28, libaan::count_leading_1(uint64_t(0xfffffff00fffffffULL)));
}

TEST(byte_hh, count_trailing_1) {
    EXPECT_EQ(4, libaan::count_trailing_1(uint32_t(0xf)));
    EXPECT_EQ(16, libaan::count_trailing_1(uint32_t(0xffff)));
    EXPECT_EQ(32, libaan::count_trailing_1(uint32_t(0xffffffff)));
    EXPECT_EQ(28, libaan::count_trailing_1(uint32_t(0xfffffff)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint32_t(0xfffffff0)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint32_t(0xf0000000)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint32_t(0xffff0000)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint32_t(0xfffffff0)));

    EXPECT_EQ(4, libaan::count_trailing_1(uint64_t(0xfULL)));
    EXPECT_EQ(16, libaan::count_trailing_1(uint64_t(0xffffULL)));
    EXPECT_EQ(32, libaan::count_trailing_1(uint64_t(0xffffffffULL)));
    EXPECT_EQ(64, libaan::count_trailing_1(uint64_t(0xffffffffffffffffULL)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint64_t(0xf000000000000000ULL)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint64_t(0xffff000000000000ULL)));
    EXPECT_EQ(0, libaan::count_trailing_1(uint64_t(0xffffffff00000000ULL)));
    EXPECT_EQ(24, libaan::count_trailing_1(uint64_t(0xffffffff00ffffffULL)));
}

/*
TEST(byte_hh, pad32_trailing_0) {
    EXPECT_EQ(0xf0000000, libaan::pad32_trailing_0(uint8_t(0xf)));
}
*/

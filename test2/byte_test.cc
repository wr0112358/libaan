#include "libaan/byte.hh"

#include <gtest/gtest.h>

TEST(byte_hh, roundtonext8) {
    std::vector<size_t> ts { 0, 1, 7, 8, 15, 16, 17, size_t((2<<31) - 1), (2<<31), (2<<31) + 1 };
    std::vector<size_t> te { 0, 8, 8, 8, 16, 16, 24, 2<<31, 2<<31, (2<<31) + 8 };
    for(size_t i = 0; i < ts.size(); i++)
        EXPECT_EQ(te[i], libaan::roundtonext8(ts[i]));
}

TEST(byte_hh, roundtolast8) {
    std::vector<size_t> ts { 0, 1, 7, 8, 15, 16, 17, size_t((2<<31) - 1), (2<<31), (2<<31) + 1 };
    std::vector<size_t> te { 0, 0, 0, 8, 8, 16, 16, size_t((2<<31) - 8), 2<<31, 2<<31 };
    for(size_t i = 0; i < ts.size(); i++)
        EXPECT_EQ(te[i], libaan::roundtolast8(ts[i]));
}

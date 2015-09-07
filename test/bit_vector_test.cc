#include <iostream>
#include "libaan/bit_vector.hh"

#include <gtest/gtest.h>

TEST(bit_vector_hh, clear_lsb_to_msb) {
    std::vector<uint64_t> in_val { 0xffff, 0xffff, 0xffff, 0xffff,
            0, 0, 0, 0,
            0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull };
    std::vector<size_t> in_i { 0, 7, 14, 15,
            0, 8, 16, 15,
            0, 30, 31, 32, 62, 63 };
    std::vector<uint64_t> exp { 0xfffe, 0xff00, 0x8000, 0,
            0, 0, 0, 0,
            0xfffffffffffffffeull, 0xffffffff80000000ull, 0xffffffff00000000ull, 0xfffffffe00000000ull, 0x8000000000000000ull, 0 };
    for(size_t i = 0; i < in_val.size(); i++)
        EXPECT_EQ(exp[i], libaan::clear_lsb_to_msb(in_val[i], in_i[i]));
}

TEST(bit_vector_hh, set_lsb_to_msb) {
    std::vector<uint64_t> in_val { 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
            0, 0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0 };
    std::vector<size_t> in_i { 0, 7, 14, 15, 16,
            0, 6, 7, 8,
            14, 15, 16,
            30, 31, 32,
            62, 63 };
    std::vector<uint64_t> exp { 0xff01, 0xffff, 0xffff, 0xffff, 0x1ffff,
            1, 0x7f, 0xff, 0x1ff,
            0x7fff, 0xffff, 0x1ffff,
            0x7fffffff, 0xffffffff, 0x1ffffffff,
            0x7fffffffffffffffull, 0xffffffffffffffffull };
    for(size_t i = 0; i < in_val.size(); i++)
        EXPECT_EQ(exp[i], libaan::set_lsb_to_msb(in_val[i], in_i[i]));
}

TEST(bit_vector_hh, clear_msb_to_lsb) {
    std::vector<size_t> in_val { 0xffff000000000000, 0xffff000000000000, 0xffff000000000000, 0xffff000000000000,
            0, 0, 0, 0,
            0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull, 0xffffffffffffffffull };
    std::vector<size_t> in_i { 0, 7, 14, 15,
            0, 8, 16, 15,
            0, 30, 31, 32, 62, 63 };
    std::vector<size_t> exp { 0x7fff000000000000, 0x00ff000000000000, 0x0001000000000000, 0,
            0, 0, 0, 0,
            0x7fffffffffffffffull, 0x00000001ffffffffull, 0x00000000ffffffffull, 0x000000007fffffffull, 1, 0 };
    for(size_t i = 0; i < in_val.size(); i++)
        EXPECT_EQ(exp[i], libaan::clear_msb_to_lsb(in_val[i], in_i[i]));
}

TEST(bit_vector_hh, set_msb_to_lsb) {
    std::vector<size_t> in_val {
            0, 0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0 };
    std::vector<size_t> in_i {
            0, 6, 7, 8,
            14, 15, 16,
            30, 31, 32,
            62, 63 };
    std::vector<size_t> exp {
            0x8000000000000000ull, 0xfe00000000000000ull, 0xff00000000000000ull, 0xff80000000000000ull,
            0xfffe000000000000ull, 0xffff000000000000ull, 0xffff800000000000ull,
            0xfffffffe00000000ull, 0xffffffff00000000ull, 0xffffffff80000000ull,
            0xfffffffffffffffeull, 0xffffffffffffffffull };

    for(size_t i = 0; i < in_val.size(); i++)
        EXPECT_EQ(exp[i], libaan::set_msb_to_lsb(in_val[i], in_i[i]));
}

TEST(bit_vector_hh, bit_vector) {
    libaan::bit_vector b63(63);
    libaan::bit_vector b64(64);
    libaan::bit_vector b1024(1024);
    EXPECT_EQ(8, b63.size());
    EXPECT_EQ(8, b64.size());
    EXPECT_EQ(1024 / 8, b1024.size());

}

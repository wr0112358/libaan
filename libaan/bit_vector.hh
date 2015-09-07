#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <utility>

namespace libaan {

// calculate number of 64 bit integers necessary to store bit_count bits
inline constexpr uint64_t u64_from_bitcount(const size_t bit_count)
{
    const size_t l = bit_count / 64;
    return (bit_count % 64 == 0) ? l : l + 1;
}

// return pair of:
//  - index of bit 'bit_nr' in 64 bit steps
//  - numer of bits to shift for mask to access bit at given index
inline constexpr std::pair<uint64_t, uint64_t> idx(const size_t bit_nr)
{
    return std::make_pair((bit_nr & ~0x3fULL) >> 6ULL, bit_nr & 0x3fULL);
}

inline uint64_t mask(const size_t bit_nr)
{
    assert(bit_nr <= 64);
    return 1ULL << bit_nr;
}

/*
operations for all bit_vector containers:
size() := size in bytes
bits_total() := size() * 8
*/

class bit_vector {
public:
    bit_vector(const size_t bit_count)
    {
        buff.resize(u64_from_bitcount(bit_count), 0);
    }

    bit_vector(const std::vector<uint64_t> &other)
        : buff(other)
    {
    }

    void set(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        buff[i.first] |= 1ULL << i.second;
    }

    void unset(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        buff[i.first] &= ~(1ULL << i.second);
    }

    uint64_t get(const size_t bit_idx) const
    {
        const auto i = idx(bit_idx);
        return buff[i.first] & (1ULL << i.second);
        //return buff.at(i.first) & (1ULL << i.second);
    }

    size_t size() const { return buff.size() * sizeof(uint64_t); }
    size_t bits_total() const { return buff.size() * sizeof(uint64_t) * 8; }
    void set_all(bool set) { memset(&buff[0], set ? 0xffu : 0, buff.size() * sizeof(uint64_t)); }

    const std::vector<uint64_t> &data() const { return buff; }
    const char *raw() const { return reinterpret_cast<const char *>(&buff[0]); }
    size_t raw_size() const { return size(); }

    friend bool operator==(const bit_vector &lhs, const bit_vector &rhs);

private:
    std::vector<uint64_t> buff;
};

inline bool operator==(const bit_vector &lhs, const bit_vector &rhs)
{
    return lhs.buff == rhs.buff;
}

class mapped_bit_vector {
public:
    mapped_bit_vector() : mapped_data(nullptr), mapped_size(0) {}
    mapped_bit_vector(char *data, const size_t size)
        : mapped_data(data), mapped_size(size) { assert(mapped_size % 8 == 0); }

    void map(char *data, const size_t size)
    {
        mapped_data = data;
        mapped_size = size;
    }

    void set(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        uint64_t *buff = reinterpret_cast<uint64_t *>(mapped_data);
        buff[i.first] |= 1ULL << i.second;
    }

    void unset(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        uint64_t *buff = reinterpret_cast<uint64_t *>(mapped_data);
        buff[i.first] &= ~(1ULL << i.second);
    }

    uint64_t get(const size_t bit_idx) const
    {
        const auto i = idx(bit_idx);
        uint64_t *buff = reinterpret_cast<uint64_t *>(mapped_data);
        return buff[i.first] & (1ULL << i.second);
        //return buff.at(i.first) & (1ULL << i.second);
    }

    size_t size() const { return mapped_size; }
    size_t bits_total() const { return mapped_size * 8; }
    void set_all(bool set) { memset(&mapped_data, set ? 0xff : 1, mapped_size); }

    const uint64_t *data() const { return reinterpret_cast<uint64_t *>(mapped_data); }
    const char *raw() const { return mapped_data; }
    size_t raw_size() const { return size(); }

    friend mapped_bit_vector &operator|=(mapped_bit_vector &lhs, const mapped_bit_vector &rhs);

private:
    char *mapped_data;
    size_t mapped_size;
};

inline mapped_bit_vector &operator|=(mapped_bit_vector &lhs,
                                     const mapped_bit_vector &rhs)
{
    assert(lhs.size() == rhs.size());
    assert(lhs.size() % 8 == 0);

    const auto sz = lhs.size() / 8;
    for(size_t i = 0; i < sz; i++)
        reinterpret_cast<uint64_t *>(lhs.mapped_data)[i]
            |= reinterpret_cast<uint64_t *>(rhs.mapped_data)[i];
    return lhs;
}

// clear/set all bits from lsb/msb to offset
uint64_t clear_lsb_to_msb(uint64_t value, size_t offset)
{
    assert(offset < 64);
    return offset == 63 ? 0 : value & ~((1ULL << (offset + 1)) - 1);
}

uint64_t clear_msb_to_lsb(uint64_t value, size_t offset)
{
    assert(offset < 64);
    return offset == 63 ? 0 : value & ~(0xffffffffffffffffull << (63 - offset));
}

uint64_t set_lsb_to_msb(uint64_t value, size_t offset)
{
    assert(offset < 64);
    return offset == 63 ? 0xffffffffffffffffull : value | ((1ULL << (offset + 1)) - 1);
}

uint64_t set_msb_to_lsb(uint64_t value, size_t offset)
{
    assert(offset < 64);
    return offset == 63 ? 0xffffffffffffffffull : value | (0xffffffffffffffffull << (63 - offset));
}

}

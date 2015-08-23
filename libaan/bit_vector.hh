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
operations for all bit_vector containers are specified as follows
size() := size in bytes
bits_total() := size() * 8
*/

template<size_t bit_count>
class bit_vector_ct {
public:
    void set(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        data[i.first] |= 1ULL << i.second;
    }

    void unset(const size_t bit_idx)
    {
        const auto i = idx(bit_idx);
        data[i.first] &= ~(1ULL << i.second);
    }

    uint64_t get(const size_t bit_idx) const
    {
        const auto i = idx(bit_idx);
        return data[i.first] & (1ULL << i.second);
        //return data.at(i.first) & (1ULL << i.second);
    }

    size_t size() const { return data.size() * sizeof(uint64_t); }
    size_t bits_total() const { return data.size() * sizeof(uint64_t) * 8; }
    void set_all(bool set) { memset(&data[0], set ? 0xffffffffffffffff : 0, data.size() * sizeof(uint64_t)); }

    const char *raw() const { return reinterpret_cast<char *>(&data[0]); }
    size_t raw_size() const { return size(); }

private:
    std::array<uint64_t, u64_from_bitcount(bit_count)> data {};
};

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
    void set_all(bool set) { memset(&buff[0], set ? 0xffffffffffffffff : 0, buff.size() * sizeof(uint64_t)); }

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

//    friend bool operator==(const bit_vector &lhs, const bit_vector &rhs);
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

// clear all bits [msb, i]
uint64_t clear_lsb_to_msb(uint64_t value, size_t i)
{
    const auto mask = (1ULL << i) - 1; // i=4 -> 10000, 10000 - 1 = 01111
    return value & ~mask; // i=4, ~mask=1...10000
}

// clear all bits [i, lsb(0)]
uint64_t clear_msb_to_lsb(uint64_t value, size_t i)
{
    const auto mask = ~((1ULL << (i+1)) - 1);
    return value & ~mask;
}

uint64_t set_lsb_to_msb(uint64_t value, size_t i)
{
    const auto mask = (1ULL << i) - 1; // i=4 -> 10000, 10000 - 1 = 01111
    return value | mask; // i=4, ~mask=1...10000
}

uint64_t set_msb_to_lsb(uint64_t value, size_t i)
{
    const auto mask = ~((1ULL << (i+1)) - 1);
    return value | mask;
}

#if 0
const size_t NONE = std::numeric_limits<size_t>::max();

template<typename bitvec_type)
size_t find_next_set(const bitvec_type &bitvec, size_t bit_off = 0)
{
}

template<typename bitvec_type)
size_t find_next_not_set(const bitvec_type &bitvec, size_t bit_off = 0)
{
    find_next_not_set(bitvec.raw(), bitvec.raw_size(), bit_off);
}

size_t find_next_not_set(const char *data, size_t len, size_t bit_off = 0)
{
    assert(bit_off < 64);
    if(bit_off == 0) {
        const auto it = std::find_if_not(data, data + len, [](const char c) { return c != 0xff; });
        if(it == data + len)
            return NONE;
    }

    const uint64_t *ptr = reinterpret_cast<const uint64_t *>(bitvec.raw());
    const size_t l = bitvec.raw_size();
    for(size_t i = 0; i + sizeof(uint64_t) <= l; i += sizeof(uint64_t)) {
        // set already visited bits
        if(bit_off % 64 != 0) {
            // set bit_off leading bits to 0
            // then count leading 0-bits
            // if(count > bit_off) return bit_off + 1
            const auto x = bit_off ? set_lsb_to_msb(*(ptr + i), bit_off - 1) : *(ptr + i);
            const auto count = count_trailing_0(x);
            if(count == 64)
                continue;
            return count + 1

        }
            memmem();
            return xyz;
    }
    return NONE;
}

template<typename bitvec_type, typename lambda_t>
void bitvec_for_each(bool set, const bitvec_type &bitvec, const lambda_t &lambda)
{
    auto find_lambda = set ? find_next_set : find_next_not_set;
    size_t bit_off = 0;
    while(true) {
        bit_off = find_lambda(bitvec, bit_off);
        if(bit_off == NONE)
            break;
        lambda(bit_off);
        bit_off++;
    }
}
#endif

}

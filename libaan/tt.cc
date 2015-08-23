#include "bit_vector.hh"
#include "byte.hh"

#include <algorithm>
#include <iostream>

template<typename lambda_t>
void for_each_set(uint64_t v, lambda_t lambda)
{
    for(size_t i = 0; i < 64; i++) if(v & (1ULL << i)) lambda(i);
}

template<typename lambda_t>
void for_each_not_set(uint64_t v, lambda_t lambda)
{
    for_each_set(~v, lambda);
}

size_t next_not_0(const uint64_t *d, size_t l, size_t off = 0)
{
    if(off >= l)
        return l;
    const auto begin = d + off;
    const auto end = d + l - off;
    const auto o = std::find_if(begin, end, [](uint64_t t) { /* std::cout << "fi(" << t << ")\n";*/ return t != 0; });

//    return (o == d + l - off) ? l : o - d;
    if(o == end)
        return l;

    const auto dist = o - d;
    return dist ? dist : 0;
}

size_t next_0(const uint64_t *d, size_t l, size_t off = 0)
{
    if(off >= l)
        return l;
    const auto begin = d + off;
    const auto end = d + l - off;
    const auto o = std::find_if(begin, end, [](uint64_t t) { return t != 0xffffffffffffffffULL; });
    if(o == end)
        return l;
    const auto dist = o - d;
    return dist ? dist : 0;
}

/*
template<typename T>
size_t next_0(const T *d, size_t l, size_t off = 0)
{
    if(off >= l)
        return l;
    const auto begin = d + off;
    const auto end = d + l - off;
    const auto o = std::find_if(begin, end, [](T t) { return t != (T)0xffffffffffffffff; });
    if(o == end)
        return l;
    const auto dist = o - begin;
    return dist ? dist : 0;
}
*/

template<typename lambda_t>
void for_each_set(const uint64_t *v, size_t l, lambda_t lambda)
{
    size_t off = 0;
    do {
        off = next_not_0(v, l, off);
        if(off == l)
            break;
        for_each_set(v[off], [lambda, off](size_t inner_idx) { lambda(off, inner_idx); });
        ++off;
    } while(true);
}

template<typename lambda_t>
void for_each_not_set(const uint64_t *v, size_t l, lambda_t lambda)
{
    size_t off = 0;
    do {
        off = next_0(v, l, off);
        if(off == l)
            break;
        for_each_not_set(v[off], [lambda, off](size_t inner_idx) { lambda(off, inner_idx); });
        ++off;
    } while(true);

}

void do_it(uint64_t v)
{
    for_each_set(v, [](size_t idx) { std::cout << idx << " "; }); std::cout << "\n";
    for_each_not_set(v, [](size_t idx) { std::cout << idx << " "; }); std::cout << "\n";
//    std::cout << "next_0 = " << next_0((char *)&v, 8) << "\n";
    std::cout << "next_0 = " << next_0(&v, 1) << "\n";
    std::cout << "next_not_0 = " << next_not_0(&v, 1) << "\n";
}

void doit2(const uint64_t *d, size_t l)
{
    std::cout << "next_not_0: " << next_not_0(d, l, 0) << "\n";
    std::cout << "next_0: " << next_0(d, l) << "\n";
}

template<typename bv_t>
void doit3(const bv_t &bv, const char *pref)
{
    std::cout << "expected(" << pref << "): ";
    for_each_set(&bv.data()[0], bv.data().size(), [](size_t o, size_t i) {
            std::cout << o * 64 + i << " "; });
    std::cout << "\n";
}

template<typename bv_t>
void doit4(const bv_t &bv, const char *pref)
{
    std::cout << "expected(" << pref << "): ";
    for_each_not_set(&bv.data()[0], bv.data().size(), [](size_t o, size_t i) {
            std::cout << o * 64 + i << " "; });
    std::cout << "\n";
}

int main()
{
    do_it(0xff);
    do_it(0xfe);
    do_it(0x8000000000000000ULL);
    do_it(0x7fffffffffffffff);

    //libaan::bit_vector bv(63333);
    libaan::bit_vector bv(2049);
    bv.set_all(false);
    doit3(bv, "none");

    libaan::bit_vector bv2(2049);
    doit3(bv2, "uninitialised");

    libaan::bit_vector bv3(2049);
    bv3.set(2048);
    doit3(bv3, "2048");

    bv.set_all(true);
    doit4(bv, "none");

    bv.set_all(true);
    doit4(bv, "none");

    bv.set_all(false);
    bv.set(1333);
    bv.set(13);
    bv.set(2048);
    doit3(bv, "1333 13 2048");

    //std::cout << bv.get(1333) << "/" << bv.get(13) << "/" << bv.get(2048) << "\n";
    bv.set_all(true);
    bv.unset(1333);
    bv.unset(13);
    bv.unset(2048);
    //std::cout << bv.get(1333) << "/" << bv.get(13) << "/" << bv.get(2048) << "\n";
    doit4(bv, "1333 13 2048");

    libaan::bit_vector bv4(10);
    doit4(bv4, "1 .. 64");


//    std::cout << "setting bits: ";
//    for(size_t i = 0; i + 1024 < bv.bits_total(); i+=1024) {
//        std::cout << i << " ";
//        bv.set(i);
//    }
    std::cout << "\n";


    uint64_t vv[] = { 1, 2, 3, 0, 1, 0, 1, 0, 1};
    doit2(vv, sizeof(vv));

    {
        libaan::bit_vector t(64);
        libaan::bit_vector t2(65);

        libaan::bit_vector t3(2048);
        libaan::bit_vector t4(2049);
        std::cout << t.data().size() << "/"
                  << t2.data().size() << "/"
                  << t3.data().size() << "/"
                  << t4.data().size() << "\n";

        std::cout << libaan::u64_from_bitcount(64) << "/" << t.bits_total() << "\n"
                  << libaan::u64_from_bitcount(65) << "/" << t2.bits_total() << "\n"
                  << libaan::u64_from_bitcount(2048) << "/" << t3.bits_total() << "\n"
                  << libaan::u64_from_bitcount(2049) << "/" << t4.bits_total() << "\n";
    }
    return 0;
}

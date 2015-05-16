#include "libaan/time.hh"

#include <gtest/gtest.h>

TEST(time_hh, to_string) {
    const auto t1 = libaan::time_point_t();
    EXPECT_EQ("01.01.1970_01.00.00", libaan::to_string(t1));
    EXPECT_EQ("01.01.1970 01:00:00", libaan::to_string(t1, false));

    const libaan::time_point_t t2 = libaan::clock_t::from_time_t(2147483647);
    EXPECT_EQ("19.01.2038 04:14:07", libaan::to_string(t2, false));
}

/*
std::time_t steady_clock_to_time_t(const std::chrono::steady_clock::time_point &t)
{
    using namespace std::chrono;
    return system_clock::to_time_t(system_clock::now() + (t - steady_clock::now()));
}

// t<0 fails
std::chrono::steady_clock::time_point steady_clock_from_time_t(std::time_t t)
{
    using namespace std::chrono;

    const auto sy = system_clock::now();
    const auto st = steady_clock::now();
    const auto delta = system_clock::from_time_t(t) - sy;
    return st + delta;
}

TEST(time_hh, steady_clock_to_time_t) {
    // "steady tp -> time_t -> system tp"
//    EXPECT_EQ(libaan::clock_t::from_time_t(0), libaan::clock_t::from_time_t(steady_clock_to_time_t(libaan::monotonic_time_point_t())));
//    EXPECT_EQ(libaan::monotonic_time_point_t(), libaan::clock_t::from_time_t(0), libaan::clock_t::from_time_t(steady_clock_to_time_t()));
    const auto tm1 = libaan::monotonic_time_point_t();
    //EXPECT_EQ(tm1, steady_clock_from_time_t(steady_clock_to_time_t(tm1)));

//    std::set<std::time_t> ts {0, 1, 2<<31, -3};
//    for(auto t: ts)
//        EXPECT_EQ(t, steady_clock_to_time_t(steady_clock_from_time_t(t)));
//    EXPECT_EQ(-1, libaan::clock_t::to_time_t(libaan::clock_t::from_time_t(-1)));
}
*/

TEST(time_hh, serialize) {
    using namespace libaan;

    std::set<std::int64_t> ts { 0, 1, 2<<31, -1, -(2<<31), 0xf000000b };
    for(auto t: ts)
        EXPECT_EQ(t, storable_time_point(deserialize_time_point<libaan::time_point_t>(t)));

/*
TODO: uncomment when fixed  
Sat Jun 20 12:20:10 CEST 2015
g++ --version
g++ (GCC) 5.1.1 20150612 (Red Hat 5.1.1-3)
https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66536

    const auto t1 = libaan::clock_t::now();
    std::set<decltype(libaan::clock_t::now())> tps { t1, {},
            t1 + std::chrono::nanoseconds(1), t1 - std::chrono::nanoseconds(199),
                libaan::clock_t::from_time_t(-1),
                libaan::clock_t::from_time_t(0),
                libaan::clock_t::from_time_t(2<<31),
                libaan::clock_t::from_time_t(-2<<31),
                };
    for(auto t: tps)
        EXPECT_EQ(t, deserialize_time_point<libaan::time_point_t>(storable_time_point(t)));

    const auto tm1 = monotonic_clock_t::now();
    std::set<decltype(libaan::monotonic_clock_t::now())> tpms { tm1, {},
            tm1 + std::chrono::nanoseconds(1), tm1 - std::chrono::nanoseconds(199) };
    for(auto t: tpms)
        EXPECT_EQ(t, deserialize_time_point<libaan::monotonic_time_point_t>(storable_time_point(t)));
*/
}

TEST(time_hh, dura_to_string) {
    const std::set<int32_t> vs {13, -13, 0, -1, 1, 2<<31, -(2<<31) };
    const auto t1 = libaan::clock_t::now();
    for(auto v: vs)
        EXPECT_EQ(std::to_string(v), libaan::dura_to_string<std::chrono::milliseconds>(t1, t1 + std::chrono::milliseconds(v)));

    const auto tm1 = libaan::monotonic_clock_t::now();
    for(auto v: vs)
        EXPECT_EQ(std::to_string(v), libaan::dura_to_string<std::chrono::milliseconds>(tm1, tm1 + std::chrono::milliseconds(v)));

    EXPECT_EQ("1", libaan::dura_to_string<std::chrono::seconds>(tm1, tm1 + std::chrono::milliseconds(1001)));
    EXPECT_EQ("1", libaan::dura_to_string<std::chrono::seconds>(tm1, tm1 + std::chrono::milliseconds(1999)));
    EXPECT_EQ("2", libaan::dura_to_string<std::chrono::seconds>(tm1, tm1 + std::chrono::milliseconds(2000)));
}

#include "libaan/string.hh"

#include "libaan/file.hh"

#include <algorithm>
#include <gtest/gtest.h>

#define WORD_FILE "/usr/share/dict/words"
std::string words;

std::vector<std::tuple<std::string, std::string, std::vector<std::string> > >test_set_str;
std::vector<std::tuple<std::string, char, std::vector<std::string> > >test_set_char;

void init()
{
    if(!words.empty())
        return;
    const auto sz = libaan::read_file(WORD_FILE, words,
                                      // make debugging faster:
                                      80);
    EXPECT_EQ(words.size(), sz);
    EXPECT_TRUE(words.size() > 0);

    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("aaa", "a", { "\0" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("  ", "", { }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("  ", " ", { "" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("abc", "b", { "a", "c" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("a a", " ", { "a", "a" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("a a", "a", { " ", "\0" }));

    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("aaa", "aa", { "a" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("abc", "abc", { "" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("a  a", "  ", { "a", "a" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("a  a  a", "  ", { "a", "a", "a" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("a  a  a  ", "  ", { "a", "a", "a", "" }));
    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("aaaa", "aa", { "" }));

    test_set_str.push_back(std::make_tuple<std::string, std::string, std::vector<std::string> >("", "", { }));


    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("aaa", 'a', { }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("aba", 'a', { "b" }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("abab", 'a', { "b", "b" }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("  ", ' ', { }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("abc", 'b', { "a", "c" }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("a a", ' ', { "a", "a" }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("a a", 'a', { " " }));
    test_set_char.push_back(std::make_tuple<std::string, char, std::vector<std::string> >("", '\0', { }));

}

void print(std::string words, size_t off, size_t count, bool no_nl = false)
{
    if(off >= words.size())
        return;
    if(count == 0)
        count = words.size() - off;
    count = std::min(words.size(), off + count);
    std::cout << words.substr(off, count) << (!no_nl ? "\n" : "");
}

void test_split3(const std::string &input, const std::string &delim,
                 const std::vector<std::string> &result)
{
    const auto r = libaan::split3(input, delim);
//    for(const auto rr: r) std::cout << "\"" << rr<< "\", "; std::cout << "\n";
    EXPECT_EQ(result.size(), r.size());
    if(result.size() != r.size())
        return;
    EXPECT_EQ(result, r);
}


TEST(string_hh, stringtype_equality) {
    init();
    for(const auto &t: test_set_str) {
        //libaan::tostringtype(std::get<0>(t));
        EXPECT_TRUE(libaan::string_type(std::get<0>(t)) == libaan::string_type(std::get<0>(t)));
        EXPECT_TRUE(std::get<0>(t) == libaan::string_type(std::get<0>(t)));
        EXPECT_TRUE(libaan::string_type(std::get<0>(t)) == std::get<0>(t));
        EXPECT_TRUE(std::get<0>(t).c_str() == libaan::string_type(std::get<0>(t)));
        EXPECT_TRUE(libaan::string_type(std::get<0>(t)) == std::get<0>(t).c_str());
        EXPECT_TRUE(std::get<0>(t).c_str() == libaan::string_type(std::get<0>(t).c_str()));
        EXPECT_TRUE(libaan::string_type(std::get<0>(t)) == std::get<0>(t).c_str());
    }
}

TEST(string_hh, split3) {
    init();

    for(const auto &t: test_set_str)
        test_split3(std::get<0>(t), std::get<1>(t), std::get<2>(t));
}

bool operator==(const std::vector<std::string> &lhs,
                const std::vector<libaan::string_type> &rhs)
{
    if(lhs.size() != rhs.size())
        return false;
    for(size_t i = 0; i < lhs.size(); i++)
        if(std::string(rhs[i]) != lhs[i])
            return false;
    return true;
}

template<typename delim_type>
void test_split2(const std::string &input, const delim_type &delim,
                const std::vector<std::string> &result,
                 bool input_as_stringtype = false)
{
    const auto r = input_as_stringtype
        ? libaan::split(input, delim)
        : libaan::split(input, delim);

//    for(const auto rr: r) std::cout << "\"" << std::string(rr) << "\", "; std::cout << "\n";
    EXPECT_EQ(result.size(), r.size());
    if(result.size() != r.size())
        return;
    EXPECT_TRUE(result == r);

    //EXPECT_EQ(result, r);
}

void test_split2_b(const std::string &input, const libaan::string_type &delim,
                   const std::vector<std::string> &result,
                   bool input_as_stringtype = false)
{
    const auto r = input_as_stringtype
        ? libaan::split(input, delim)
        : libaan::split2(input, delim);

//    for(const auto rr: r) std::cout << "\"" << std::string(rr) << "\", "; std::cout << "\n";
    EXPECT_EQ(result.size(), r.size());
    if(result.size() != r.size())
        return;
    EXPECT_TRUE(result == r);

    //EXPECT_EQ(result, r);
}

void test_split2_c(const libaan::string_type &input, const libaan::string_type &delim,
                   const std::vector<std::string> &result)
{
    const auto r = libaan::split(input, delim);

//    for(const auto rr: r) std::cout << "\"" << std::string(rr) << "\", "; std::cout << "\n";
    EXPECT_EQ(result.size(), r.size());
    if(result.size() != r.size())
        return;
    EXPECT_TRUE(result == r);

    //EXPECT_EQ(result, r);
}

TEST(string_hh, split2_str) {
    init();

    for(const auto &t: test_set_str)
        test_split2(std::get<0>(t), std::get<1>(t), std::get<2>(t));

    for(const auto &t: test_set_str)
        test_split2_b(std::get<0>(t), libaan::string_type(std::get<1>(t)), std::get<2>(t));
    for(const auto &t: test_set_str)
        test_split2_c(libaan::string_type(std::get<0>(t)), libaan::string_type(std::get<1>(t)), std::get<2>(t));

    for(const auto &t: test_set_str)
        test_split2(std::get<0>(t), std::get<1>(t), std::get<2>(t), true);
}

TEST(string_hh, split2_char) {
    init();

    for(const auto &t: test_set_char)
        test_split2(std::get<0>(t), std::get<1>(t), std::get<2>(t));

    for(const auto &t: test_set_char)
        test_split2(std::get<0>(t), std::get<1>(t), std::get<2>(t), true);
    for(const auto &t: test_set_char)
        test_split2(std::get<0>(t), std::get<1>(t), std::get<2>(t), true);
}

TEST(string_hh, to_hex_string) {
    init();
}

TEST(string_hh, stl_search_all) {
    init();
    std::string patt {"1080" };
    auto r = libaan::search::stl_search_all(patt, words);
    EXPECT_EQ(r.size(), 1);
    if(r.empty())
        exit(EXIT_FAILURE);
    EXPECT_EQ(r.front(), 0);

    patt.assign("10th");
    r = libaan::search::stl_search_all(patt, words);
    EXPECT_EQ(r.size(), 1);
    if(r.empty())
        exit(EXIT_FAILURE);
    EXPECT_EQ(r.front(), 14);

    const std::string i1 { "abcdefabc" };
    r = libaan::search::stl_search_all("abc", i1);
    EXPECT_EQ(r.size(), 2);
    if(r.empty())
        exit(EXIT_FAILURE);
    EXPECT_EQ(r[0], 0);
    EXPECT_EQ(r[1], 6);

    r = libaan::search::stl_search_all("def", i1);
    EXPECT_EQ(r.size(), 1);
    if(r.empty())
        exit(EXIT_FAILURE);
    EXPECT_EQ(r[0], 3);

    r = libaan::search::stl_search_all("def", "");
    EXPECT_TRUE(r.empty());

    r = libaan::search::stl_search_all("", "abc");
    EXPECT_TRUE(r.empty());

    // print(words, 0, 80);
    // for(auto rr: r) print(words, rr, patt.length() + 3);
}

/*
TODO: sarr*::get_suffixes()
// my benchmark programm:
// suffix array for: "banana$"       6 5 3 1 0 4 2
// suffix array for: "banana"        5 3 1 0 4 2
// suffix array for: "mississippi$"  11 10 7 4 1 0 9 8 6 3 5 2
// suffix array for: "mississippi"   10 7 4 1 0 9 8 6 3 5 2
// suffix array for: "abracadabra$"  11 10 7 0 3 5 8 1 4 6 9 2
// suffix array for: "abracadabra"   10 7 0 3 5 8 1 4 6 9 2
// suffix array for: "yabbadabbado$" 12 1 6 4 9 3 8 2 7 5 10 11 0
// suffix array for: "yabbadabbado"  1 6 4 9 3 8 2 7 5 10 11 0

*/

TEST(string_hh, sarr_cx11) {
    init();

    {
        libaan::search::sarr_cx11 sarr(words);
        auto r = sarr.search("1080");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 0);

        r = sarr.search("10th");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 14);
    }

    {
        const char * in = "abcdefabc";
        libaan::search::sarr_cx11 sarr(in, strlen(in));
        //libaan::search::sarr_cx11 sarr("abcdefabc");
        //sarr.print();
        auto r = sarr.search("abc");
        EXPECT_EQ(r.size(), 2);

        if(r.size() != 2)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 6);

        r = sarr.search("def");
        EXPECT_EQ(r.size(), 1);
        if(r.size() != 1)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 3);
    }

    {
        const char *in = "";
        libaan::search::sarr_cx11 sarr(in, std::strlen(in));
        auto r = sarr.search("def");
        EXPECT_TRUE(r.empty());
    }

    {
        const char *in = "abc";
        libaan::search::sarr_cx11 sarr(in, std::strlen(in));
        auto r = sarr.search("");
        EXPECT_TRUE(r.empty());
    }

    {
        const char *in = " ";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("  ");
        EXPECT_TRUE(r.empty());
    }

    {
        const char *in = "  ";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search(" ");

        EXPECT_EQ(r.size(), 2);
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const char *in = "aa";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("a");
        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const char *in = "aaa";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("aa");
        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }
}

TEST(string_hh, sarr_c) {
    init();

    {
        libaan::search::sarr_c sarr(words.c_str(), words.length());
        auto r = sarr.search("1080");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 0);

        r = sarr.search("10th");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 14);
    }

    {
        const char * in = "abcdefabc";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        //sarr.print();
        auto r = sarr.search("abc");

        EXPECT_EQ(r.size(), 2);
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 6);


        r = sarr.search("def");
        EXPECT_EQ(r.size(), 1);
        if(r.size() != 1)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 3);
    }

    {
        const char *in = "";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("def");
        EXPECT_TRUE(r.empty());
    }

    {
        const char *in = "abc";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("");
        EXPECT_TRUE(r.empty());
    }

    {
        const char *in = " ";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("  ");

        EXPECT_TRUE(r.empty());

    }

    {
        const char *in = "  ";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search(" ");

        EXPECT_EQ(r.size(), 2);
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const char *in = "aa";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("a");

        //std::cout << "input: "; print(in, 0, 80);
        //size_t i = 0;
        //for(auto rr: r) { std::cout << i++ << "(c): \""; print(" ", rr, 0, true); std::cout << "\"\n"; }


        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const char *in = "aaa";
        libaan::search::sarr_c sarr(in, std::strlen(in));
        auto r = sarr.search("aa");
        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }
}

TEST(string_hh, sarr_dc3) {
    init();

    {
        libaan::search::sarr_dc3 sarr(words);
        auto r = sarr.search("1080");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 0);

        r = sarr.search("10th");
        EXPECT_EQ(r.size(), 1);
        if(r.empty())
            exit(EXIT_FAILURE);
        EXPECT_EQ(r.front(), 14);
    }

    {
        const std::string in = "abcdefabc";
        libaan::search::sarr_dc3 sarr(in);
        //sarr.print();
        auto r = sarr.search("abc");

        EXPECT_EQ(r.size(), 2);
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 6);


        r = sarr.search("def");
        EXPECT_EQ(r.size(), 1);
        if(r.size() != 1)
            exit(EXIT_FAILURE);
        EXPECT_EQ(r[0], 3);
    }

    {
        const std::string in = "";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search("def");
        EXPECT_TRUE(r.empty());
    }

    {
        const std::string in = "abc";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search("");
        EXPECT_TRUE(r.empty());
    }

    {
        const std::string in = " ";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search("  ");

        EXPECT_TRUE(r.empty());
    }

    {
        const std::string in = "  ";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search(" ");

        //std::cout << "input: "; print(" ", 0, 80);
        //size_t i = 0;
        //for(auto rr: r) { std::cout << i++ << "(dc3): \""; print(" ", rr, 0, true); std::cout << "\"\n"; }

        EXPECT_EQ(r.size(), 2);
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const std::string in = "aa";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search("a");

        //std::cout << "input: "; print(in, 0, 80);
        //size_t i = 0;
        //for(auto rr: r) { std::cout << i++ << "(dc3): \""; print(" ", rr, 0, true); std::cout << "\"\n"; }

        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }

    {
        const std::string in = "aaa";
        libaan::search::sarr_dc3 sarr(in);
        auto r = sarr.search("aa");
        EXPECT_EQ(2, r.size());
        if(r.size() != 2)
            exit(EXIT_FAILURE);
        std::sort(std::begin(r), std::end(r));
        EXPECT_EQ(r[0], 0);
        EXPECT_EQ(r[1], 1);
    }
}

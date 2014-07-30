// http://www.geeksforgeeks.org/suffix-array-set-1-introduction/

/*
Benchmark example run:
$ ./benchmark_O3 -f ../../../git-repos/libaan/test/words.test 
Pattern("nan") found at index 235409
Pattern("nan") found at index 235409 -> "aaa
Pattern("nan") found at index 235409
Suffix Search for "nan" in file with size 3206080:
    oldschool_suffixarray_search_time: 0.07951 ms    1159 results
    oldschool_suffixarray_build_time: 1226.3 ms
    -> sum = 1226.37ms
    stl_suffixarray_search_time: 0.081049 ms    1159 results
    stl_suffixarray_build_time: 1263.27 ms
    -> sum = 1263.35ms
    stl_search_time: 11.6095 ms    1159 results
    dc3_suffixarray_search_time: 0.079009 ms    1159 results
    dc3_suffixarray_build_time: 993.676 ms
    -> sum = 993.755ms
    io_time: 2.81619 ms

$ ./benchmark_debug -f ../../../git-repos/libaan/test/words.test
Pattern("nan") found at index 235409
Pattern("nan") found at index 235409 -> "aaa
Pattern("nan") found at index 235409
Suffix Search for "nan" in file with size 3206080:
    oldschool_suffixarray_search_time: 0.14138 ms    1159 results
    oldschool_suffixarray_build_time: 1887.94 ms
    -> sum = 1888.08ms
    stl_suffixarray_search_time: 0.132424 ms    1159 results
    stl_suffixarray_build_time: 2551.95 ms
    -> sum = 2552.09ms
    stl_search_time: 9.81733 ms    1159 results
    dc3_suffixarray_search_time: 0.165537 ms    1159 results
    dc3_suffixarray_build_time: 1471.06 ms
    -> sum = 1471.23ms
    io_time: 2.51422 ms


log(3206080) = 6.51
Time complexity to build suffix array is O(n * n * log(n)) for a O(n * log(n))
sorting algorithm.

The sorting step itself takes O(n * n * log(n)).

*/

/*
theory:
"You can compute the suffix array in linear time with the DC-3 Algorithm." [1]

[1] http://cs.stackexchange.com/questions/9447/algorithm-for-building-a-suffix-array-in-time-on-log2-n
 */

#include <iostream>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include "../chrono_util.hh"
#include "../file_util.hh"
#include "../string_suffix_search.hh"
#include "../string_suffix_search_dc3.hh"

void find_diff(const std::string &, const std::vector<std::size_t> &v1,
               const std::vector<std::size_t> &v2)
{
    for(std::size_t i = 0; i < v1.size(); i++)
        if(v1[i] != v2[i]) {
            std::string tmp;
            std::cerr << "First difference at position " << i << ": " << v1[i]
                      << " != " << v2[i] << "\n";
            // return;
        }
}

void test_for_equality(const std::string &input,
                       const std::vector<std::size_t> &v1,
                       const std::vector<std::size_t> &v2,
                       const std::vector<std::size_t> &v3,
                       const std::vector<std::size_t> &v4)
{
    if(v1.size() != v2.size() || v2.size() != v3.size()) {
        std::cerr << "\nsizes of results differ.\n\n";
        return;
    }
    find_diff(input, v1, v2);
    find_diff(input, v2, v3);
    find_diff(input, v3, v4);

    if(!std::equal(std::begin(v1), std::end(v1), std::begin(v2)))
        std::cerr << "\nv1 != v2\n\n";
    else
        std::cerr << "\nv1 == v2\n\n";

    if(!std::equal(std::begin(v2), std::end(v2), std::begin(v3)))
        std::cerr << "\nv2 != v3\n\n";
    else
        std::cerr << "\nv2 == v3\n\n";

    if(!std::equal(std::begin(v3), std::end(v3), std::begin(v4)))
        std::cerr << "\nv3 != v4\n\n";
    else
        std::cerr << "\nv3 == v4\n\n";
}

void run(const char *file, const std::string &pattern = "nan")
{
    libaan::util::time_me_ns timer;
    // load file
    std::string buff;
    libaan::util::file::read_file(file, buff);
    const auto io_time = timer.duration();

    // stl version
    timer.restart();
    std::vector<std::size_t> stl_matches;
    stl_search_all(pattern, buff, stl_matches);
    const auto stl_search_time = timer.duration();

    // build stl powered suffix array
    timer.restart();
    std::vector<std::size_t> suffixes;
    cxx11::buildSuffixArray(buff, suffixes);
    const auto stl_suffixarray_build_time = timer.duration();

    // search with stl powered suffix array
    timer.restart();
    std::vector<std::size_t> stl_suffixarray_matches;
    cxx11::search(pattern, buff, suffixes, stl_suffixarray_matches);
    const auto stl_suffixarray_search_time = timer.duration();

    // build oldschool suffix array
    timer.restart();
    int *suffixArr = oldschool::buildSuffixArray(buff.c_str(), buff.length());
    const auto oldschool_suffixarray_build_time = timer.duration();

    // search with oldschool suffix array
    timer.restart();
    std::vector<std::size_t> oldschool_suffixarray_matches;
    oldschool::search(pattern.c_str(), buff.c_str(), suffixArr, buff.length(),
                      oldschool_suffixarray_matches);
    const auto oldschool_suffixarray_search_time = timer.duration();

    // build dc3 suffix array
    timer.restart();
    dc3_api::suffix_array_dc3 dc3_lookup(buff);
    const auto dc3_suffixarray_build_time = timer.duration();

    // search with dc3 suffix array
    timer.restart();
    std::vector<std::size_t> dc3_suffixarray_matches;
    dc3_lookup.search(pattern, dc3_suffixarray_matches);
    const auto dc3_suffixarray_search_time = timer.duration();

    const auto time_mod = 1. / (1000. * 1000.);
    std::cout << "Suffix Search for \"" << pattern << "\" in file with size "
              << buff.length() << ":\n"
              << "    oldschool_suffixarray_search_time: "
              << oldschool_suffixarray_search_time * time_mod << " ms    "
              << oldschool_suffixarray_matches.size() << " results\n"

              << "    oldschool_suffixarray_build_time: "
              << oldschool_suffixarray_build_time * time_mod << " ms\n"
              << "    -> sum = "
              << (oldschool_suffixarray_build_time
                  + oldschool_suffixarray_search_time) * time_mod << "ms\n"

              << "    stl_suffixarray_search_time: "
              << stl_suffixarray_search_time * time_mod << " ms    "
              << stl_suffixarray_matches.size() << " results\n"

              << "    stl_suffixarray_build_time: "
              << stl_suffixarray_build_time * time_mod << " ms\n"
              << "    -> sum = "
              << (stl_suffixarray_build_time + stl_suffixarray_search_time)
                 * time_mod << "ms\n"

              << "    stl_search_time: " << stl_search_time * time_mod
              << " ms    " << stl_matches.size() << " results\n"

              << "    dc3_suffixarray_search_time: "
              << dc3_suffixarray_search_time * time_mod << " ms    "
              << dc3_suffixarray_matches.size() << " results\n"

              << "    dc3_suffixarray_build_time: "
              << dc3_suffixarray_build_time * time_mod << " ms\n"
              << "    -> sum = "
              << (dc3_suffixarray_build_time + dc3_suffixarray_search_time)
                 * time_mod << "ms\n"

              << "    io_time: " << io_time * time_mod << " ms\n";

    std::sort(std::begin(stl_matches), std::end(stl_matches));
    std::sort(std::begin(stl_suffixarray_matches),
              std::end(stl_suffixarray_matches));
    std::sort(std::begin(oldschool_suffixarray_matches),
              std::end(oldschool_suffixarray_matches));
    std::sort(std::begin(dc3_suffixarray_matches),
              std::end(dc3_suffixarray_matches));

    test_for_equality(buff, stl_matches, stl_suffixarray_matches,
                      oldschool_suffixarray_matches, dc3_suffixarray_matches);
}

void create(const std::string &in)
{
    std::cout << "suffix array for: \"" << in << "\"\n";
    std::vector<std::size_t> suffixes;
    cxx11::buildSuffixArray(in, suffixes);
    int *suffixArr = oldschool::buildSuffixArray(in.c_str(), in.length());

    for(auto s : suffixes)
        std::cout << s << " ";
    std::cout << "\n\n";

    for(size_t i = 0; i < in.length(); i++)
        std::cout << suffixArr[i] << " ";
    std::cout << "\n";
}

// Driver program to test above functions
int main(int argc, char *argv[])
{
    if(argc == 3) {
        const std::string a1(argv[1]);
        if(a1 == "-f")
            run(argv[2]);
        else if(a1 == "-s")
            create(argv[2]);
    } else {
        std::cout << "provide either \"-f input_text_file\" or \"-s "
                     "input_string\" as arg.\n";
        return -1;
    }

    return 0;
}

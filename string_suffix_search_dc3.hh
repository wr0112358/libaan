#ifndef _STRING_SUFFIX_SEARCH_DC3_HH_
#define _STRING_SUFFIX_SEARCH_DC3_HH_

/*
Implementation of DC3 algorithm for suffix array construction[1].

[1] Kärkkäinen, Juha; Sanders, Peter (2003). "Simple Linear Work Suffix Array
    Construction". Automata, Languages and Programming. Lecture Notes in
    Computer Science 2719. p. 943.
*/

/*
Example benchmark showed timings:

create_source_array: 0.000279 ms
create_suffix_array_buffer: 0.000254 ms
suffixArray: 0.003587 ms
suffix array for "yabbadabbado$":
12 1 6 4 9 3 8 2 7 5 10 11 0 

2. input string with 3206080 bytes
create_source_array: 15.6788 ms
create_suffix_array_buffer: 0.012683 ms
suffixArray: 1069.48 ms

Summary:
Having to copy the input string into an int array of the same size
is not optimal, but it is negligible in comparison to the overall
building times.

*/
#include "string_suffix_search_dc3_internal.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#ifdef _DEBUG_TIMINGS_
#include "chrono_util.hh"
#endif

namespace dc3_api
{

class suffix_array_dc3
{
public:
    suffix_array_dc3(const std::string &input_txt)
        : input_text(input_txt), max_key(-1)
    {
        create();
    }

    // Iterative search is not feasible atm. searching all occurences at
    // once might be faster than a few single searches anyway.
    //    bool search(const std::string &pattern, size_t &found_offset,
    //                size_t offset = 0)
    //    { return false; }

    // Perform binary search in suffix array. On success matches contains all
    // indices in input_text where pattern can be found.
    // Time Complexity:
    // O(m * log(n))
    // Where m is the length of the pattern and n is the length of the input
    // text.
    bool search(const std::string &pattern, std::vector<std::size_t> &matches);
    void search_and_dump_all(const std::string &pattern);
    void dump_suffix_array();

private:
    void create();

private:
    const std::string &input_text;
    // suffix_array length = input_text.length() + 3
    std::unique_ptr<int[]> suffix_array;
    // input_text_padded length = input_text.length() + 3
    std::vector<int> input_text_padded;
    int max_key;
};

// TODO: make these 2 helper functions private static?
inline void create_source_array(const std::string &in, std::vector<int> &out,
                                int &max_key);

// input_text is expected to be the output of create_source_array() function and
// must have a number of elements equal to the length of the input text padded
// with 3 additional zero entries.
inline std::unique_ptr<int[]>
create_suffix_array_buffer(const std::vector<int> &input_text_padded);
}

inline void dc3_api::create_source_array(const std::string &in,
                                         std::vector<int> &out, int &max_key)
{
    max_key = -1;
    const auto n = in.length();
    out.reserve(n + 3);
    for(size_t i = 0; i < n; i++) {
        out.push_back((int)in[i]);
        max_key = max_key > in[i] ? max_key : in[i];
    }

    // dc3 algorithm expects the last 3 elements to be 0.
    out.push_back(0);
    out.push_back(0);
    out.push_back(0);
}

// input_text is expected to be the output of create_source_array() function and
// must have a number of elements equal to the length of the input text padded
// with 3 additional zero entries.
inline std::unique_ptr<int[]>
dc3_api::create_suffix_array_buffer(const std::vector<int> &input_text_padded)
{
    const auto n = input_text_padded.size();
    std::unique_ptr<int[]> suffix_array_buffer(new int[n]);

    // dc3 algorithm expects the last 3 elements to be 0.
    suffix_array_buffer[n - 3] = suffix_array_buffer[n - 2]
        = suffix_array_buffer[n - 1] = 0;
    return suffix_array_buffer;
}

inline bool dc3_api::suffix_array_dc3::search(const std::string &pattern,
                                              std::vector<std::size_t> &matches)
{
    const int pattern_length = pattern.length();

    // binary search
    std::size_t left = 0;
    std::size_t right = input_text.length() - 1;
    while(left <= right) {
        // See if 'pattern' is prefix of middle suffix in suffix array
        std::size_t mid = left + (right - left) / 2;
        auto res = std::strncmp(pattern.c_str(),
                                input_text.c_str() + suffix_array[mid],
                                pattern_length);
        if(res == 0) {
            std::cout << "Pattern(\"" << pattern << "\") found at index "
                      << suffix_array[mid] << "\n";

            matches.push_back(suffix_array[mid]);
            int idx = mid;
            do {
                res = std::strncmp(pattern.c_str(),
                                   input_text.c_str() + suffix_array[++idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffix_array[idx]);
            } while(res == 0);
            idx = mid;
            // TODO: result is unsorted..
            do {
                res = std::strncmp(pattern.c_str(),
                                   input_text.c_str() + suffix_array[--idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffix_array[idx]);
            } while(res == 0);

            return true;
        }

        // Move to left half if pattern is alphabtically less than
        // the mid suffix
        if(res < 0)
            right = mid - 1;
        else    // Otherwise move to right half
            left = mid + 1;
    }

    // We reach here if return statement in loop is not executed
    std::cout << "Pattern not found";
    return false;
}

inline void
dc3_api::suffix_array_dc3::search_and_dump_all(const std::string &pattern)
{
    std::vector<size_t> matches;
    if(!search(pattern, matches)) {
        std::cout << "No matches for pattern \"" << pattern << "\"\n";
        return;
    }
    std::cout << matches.size() << " matches for pattern \"" << pattern
              << "\"\n";
}

inline void dc3_api::suffix_array_dc3::dump_suffix_array()
{
    if(!suffix_array)
        return;

    const auto n = input_text.length();
    std::cout << "suffix array for \""
              << (n > 32 ? input_text.substr(0, 32).replace(28, 3, ".")
                         : input_text) << "\":\n";

    for(size_t i = 0; i < n; i++)
        std::cout << suffix_array[i] << " ";
    std::cout << "\n";
}

inline void dc3_api::suffix_array_dc3::create()
{
#ifdef _DEBUG_TIMINGS_
    libaan::util::time_me_ns timer;
#endif

    create_source_array(input_text, input_text_padded, max_key);

#ifdef _DEBUG_TIMINGS_
    const auto time1 = timer.duration();
    timer.restart();
#endif

    suffix_array = create_suffix_array_buffer(input_text_padded);

#ifdef _DEBUG_TIMINGS_
    const auto time2 = timer.duration();
    timer.restart();
#endif

    suffixArray(&input_text_padded[0], suffix_array.get(), input_text.length(),
                max_key);

#ifdef _DEBUG_TIMINGS_
    const auto time3 = timer.duration();

    const auto time_mod = 1. / (1000. * 1000.);
    std::cout << "create_source_array: " << time1 *time_mod << " ms\n"
              << "create_suffix_array_buffer: " << time2 *time_mod << " ms\n"
              << "suffixArray: " << time3 *time_mod << " ms\n";
#endif
}

#endif

#include "string_suffix_search.hh"

#include <cstring>
#include <algorithm>
#include <string>
#include <iostream>

void stl_search_all(const std::string &pattern, const std::string &txt,
                    std::vector<std::size_t> &matches)
{
    std::size_t match = 0;
    while(true) {
        match = txt.find(pattern, match);
        if(match == std::string::npos)
            return;
        matches.push_back(match);
        ++match;
    }
}

namespace oldschool
{
struct suffix
{
    int index;
    const char *suff;
};

int cmp(struct suffix a, struct suffix b)
{
    return strcmp(a.suff, b.suff) < 0 ? 1 : 0;
}

void search(const char *pat, const char *txt, int *suffArr, int n,
            std::vector<std::size_t> &matches)
{
    const int pat_len
        = strlen(pat);    // get length of pattern, needed for strncmp()

    // Do simple binary search for the pat in txt using the
    // built suffix array
    int l = 0, r = n - 1;    // Initilize left and right indexes
    while(l <= r) {
        // See if 'pat' is prefix of middle suffix in suffix array
        int mid = l + (r - l) / 2;
        int res = strncmp(pat, txt + suffArr[mid], pat_len);

        // If match found at the middle, print it and return
        if(res == 0) {
            std::cout << "Pattern(\"" << pat << "\") found at index "
                      << suffArr[mid] << " -> \""
                      << std::string(txt + res, pat_len) << "\n";
            matches.push_back(suffArr[mid]);
            int idx = mid;
            do {
                res = strncmp(pat, txt + suffArr[++idx], pat_len);
                if(res != 0)
                    break;
                matches.push_back(suffArr[idx]);
            } while(res == 0);
            idx = mid;
            // TODO: result is unsorted..
            do {
                res = strncmp(pat, txt + suffArr[--idx], pat_len);
                if(res != 0)
                    break;
                matches.push_back(suffArr[idx]);
            } while(res == 0);
            return;
        }

        // Move to left half if pattern is alphabtically less than
        // the mid suffix
        if(res < 0)
            r = mid - 1;
        else    // Otherwise move to right half
            l = mid + 1;
    }

    // We reach here if return statement in loop is not executed
    std::cout << "Pattern not found";
}

int *buildSuffixArray(const char *txt, int n)
{
    // A structure to store suffixes and their indexes
    struct suffix *suffixes = new suffix[n];

    // Store suffixes and their indexes in an array of structures.
    // The structure is needed to sort the suffixes alphabatically
    // and maintain their old indexes while sorting
    for(int i = 0; i < n; i++) {
        suffixes[i].index = i;
        suffixes[i].suff = (txt + i);
    }

    // Sort the suffixes using the comparison function
    // defined above.
    std::sort(suffixes, suffixes + n, cmp);

    // Store indexes of all sorted suffixes in the suffix array
    int *suffixArr = new int[n];
    for(int i = 0; i < n; i++)
        suffixArr[i] = suffixes[i].index;

    // Return the suffix array
    return suffixArr;
}
}

namespace cxx11
{
// O(m * log(n))
void search(const std::string &pattern, const std::string &txt,
            const std::vector<std::size_t> &suffixes,
            std::vector<std::size_t> &matches)
{
    const int pattern_length = pattern.length();

    // binary search
    std::size_t left = 0;
    std::size_t right = txt.length() - 1;
    while(left <= right) {
        // See if 'pattern' is prefix of middle suffix in suffix array
        std::size_t mid = left + (right - left) / 2;
        auto res = std::strncmp(pattern.c_str(), txt.c_str() + suffixes[mid],
                                pattern_length);
        if(res == 0) {
            std::cout << "Pattern(\"" << pattern << "\") found at index "
                      << suffixes[mid] << "\n";

            matches.push_back(suffixes[mid]);
            int idx = mid;
            do {
                res = std::strncmp(pattern.c_str(),
                                   txt.c_str() + suffixes[++idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);
            idx = mid;
            // TODO: result is unsorted..
            do {
                res = std::strncmp(pattern.c_str(),
                                   txt.c_str() + suffixes[--idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);

            return;
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
}

void buildSuffixArray(const std::string &txt,
                      std::vector<std::size_t> &suffixes)
{
    const auto len = txt.length();
    suffixes.resize(len);

    std::iota(std::begin(suffixes), std::end(suffixes), 0);

    /*
        for(const auto &suffix_idx: suffixes)
            std::cout << "\"" << suffix_idx << "\" ";
        std::cout << "\n";
    */

    /*
    for(std::size_t idx = 0; idx < len; idx++)
        suffixes[idx] = idx;
        //suffixes[i].suff = (txt + i);
    */

    struct cmp_t
    {
        cmp_t(const std::string &text) : txt(text) {}
        bool operator()(std::size_t lhs, std::size_t rhs)
        {
            return std::strcmp(txt.c_str() + lhs, txt.c_str() + rhs) < 0;
        }
        const std::string &txt;
    } cmp(txt);

    std::sort(std::begin(suffixes), std::end(suffixes), cmp);
}

// A utility function to print an array of given size
void print(const std::string &txt, std::vector<std::size_t> &suffixes)
{
    std::size_t cnt = 0;
    // std::cout << "suffix array for:\n\"" << txt << "\"\n";
    for(const auto &suffix_idx : suffixes)
        std::cout << cnt++ << ": \"" << txt.c_str() + suffix_idx << "\"\n";
    std::cout << "\n";
}
}

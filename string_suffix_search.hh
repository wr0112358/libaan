#ifndef _STRING_SUFFIX_SEARCH_HH_
#define _STRING_SUFFIX_SEARCH_HH_

#include <string>
#include <vector>

// http://en.wikipedia.org/wiki/Suffix_array
// https://sites.google.com/site/yuta256/sais

// stl sequential search
void stl_search_all(const std::string &pattern, const std::string &txt,
                    std::vector<std::size_t> &matches);

namespace oldschool
{

// O(m * log(n)) <- according to webpage
// A suffix array based search function to search a given pattern
// 'pat' in given text 'txt' using suffix array suffArr[]
void search(const char *pat, const char *txt, int *suffArr, int n,
            std::vector<std::size_t> &matches);

// This is the main function that takes a string 'txt' of size n as an
// argument, builds and return the suffix array for the given string
int *buildSuffixArray(const char *txt, int n);

// A utility function to print an array of given size
void printArr(int arr[], int n, const char *txt, int txt_len);
}

namespace cxx11
{
void search(const std::string &pattern, const std::string &txt,
            const std::vector<std::size_t> &suffixes,
            std::vector<std::size_t> &matches);
void buildSuffixArray(const std::string &txt,
                      std::vector<std::size_t> &suffixes);
// A utility function to print an array of given size
void print(const std::string &txt, std::vector<std::size_t> &suffixes);
}

#endif

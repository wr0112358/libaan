#ifndef _LIBAAN_STRING_HH_
#define _LIBAAN_STRING_HH_

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cassert>

namespace libaan {

// TODO: passing string_type to a function taking a "const std::string &" should be prohibited
class string_type {
public:
    /*explicit*/ string_type(const std::string &s)
        : s(s.c_str()), l(s.length()) { }
    /*explicit*/ string_type(const char *s)
        : s(s), l(std::strlen(s)) { }
    string_type(const char *s, size_t l)
        : s(s), l(l) { }

    operator std::string() const { return std::string(s, s + l); }
    // return { s, s + l };

    size_t length() const { return l; }
    const char *data() const { return s; }

    int compare(size_t pos, size_t count, const string_type &str) const
    {
        return (str.length() < count || l < (pos + count))
                               ? false : strncmp(s + pos, str.s, count) == 0;
    }

private:
    friend bool operator==(const string_type &lhs, const string_type &rhs);
    friend bool operator==(const char *lhs, const string_type &rhs);
    friend bool operator==(const std::string &lhs, const string_type &rhs);
    friend std::vector<string_type> split(const string_type &input, const string_type &delim);
    friend std::vector<string_type> split(const string_type &input, char delim);

private:
    const char *s;
    size_t l;
};

bool operator==(const string_type &lhs, const string_type &rhs);
bool operator==(const char *lhs, const string_type &rhs);
bool operator==(const std::string &lhs, const string_type &rhs);

std::vector<string_type> split(const string_type &input, const string_type &delim);
std::vector<string_type> split(const string_type &input, char delim);

std::vector<string_type> split2(const std::string &input, const std::string &delim);

template<typename T>
inline std::string to_hex_string(const T &value)
{
    std::ostringstream os;
    os << std::hex << value << std::dec;
    return os.str();
}

namespace search {

// http://en.wikipedia.org/wiki/Suffix_array
// https://sites.google.com/site/yuta256/sais


std::vector<std::size_t> stl_search_all(const std::string &pattern, const std::string &txt);


// suffix array implementations


// TODO: passing a 'const char *' to a constructor taking a 'const std::string &'
//       is very annoying and hard to find.
// asan finds it!
// explicit does not help here. Would only help if std::string constructor was explicit
struct sarr_cx11 {
    sarr_cx11(const std::string &in) : sarr_cx11(in.c_str(), in.length()) {}
    sarr_cx11(const char *in, size_t l);
    std::vector<std::size_t> search(const std::string &pattern);
    void print();

#ifdef UNITTEST
    const std::vector<std::size_t> &get_suffixes() const { return suffixes; }
#endif

private:
    std::vector<std::size_t> suffixes;
    const char *input;
    const size_t input_length;
};

struct sarr_c {

    // This is the main function that takes a string 'txt' of size n as an
    // argument, builds and return the suffix array for the given string
    sarr_c(const char *txt, int n);
    sarr_c(const std::string &txt) : sarr_c(txt.c_str(), txt.length()) {}

    // O(m * log(n)) <- according to webpage
    // A suffix array based search function to search a given pattern
    // 'pat' in given text 'txt' using suffix array suffArr[]
    std::vector<std::size_t> search(const char *pat);

    // A utility function to print an array of given size
    void print();

#ifdef UNITTEST
    const std::vector<int> &get_suffixes() const { return suffixes; }
#endif

private:
    std::vector<int> suffixes;
    const char *txt;
    const int n;
};


struct sarr_dc3 {
    sarr_dc3(const std::string &input_txt)
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
    std::vector<std::size_t> search(const std::string &pattern);
    void search_and_dump_all(const std::string &pattern);
    void dump_suffix_array();

#ifdef UNITTEST
    const std::pair<int *, size_t> &get_suffixes() const { return std::make_pair(suffix_array, input_text_padded.size()); }
#endif

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

}

template<typename string_t, typename string2_t>
inline bool startswith(const string_t &s, const string2_t &prefix)
{
    return s.length() < prefix.length() ? false : s.compare(0, prefix.length(), prefix) == 0;
}

template<typename string_t, typename string2_t>
inline bool endswith(const string_t &s, const string2_t &suffix)
{
    return s.length() < suffix.length() ? false : s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

}

#endif

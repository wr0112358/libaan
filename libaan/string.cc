#include "string.hh"

#include <algorithm>
#include <iostream>

bool libaan::operator==(const string_type &lhs, const string_type &rhs)
{
    return lhs.l == rhs.l ? std::strncmp(lhs.s, rhs.s, lhs.l) == 0  : false;
}

bool libaan::operator==(const char *lhs, const string_type &rhs)
{
    return std::strncmp(lhs, rhs.s, rhs.l) == 0 ;
}

bool libaan::operator==(const std::string &lhs, const string_type &rhs)
{
    return lhs.size() == rhs.l ? std::strncmp(lhs.data(), rhs.s, rhs.l) == 0 : false;
}

std::vector<std::string> libaan::split3(const std::string &input,
                                       const std::string &delim)
{
    std::vector<std::string> tokens;
    if(delim.empty())
        return tokens;

    std::string::size_type start = 0;
    std::string::size_type end;

    for(;;) {
        end = input.find(delim, start);
        const auto l = end - start;
        if(l > 0)
            tokens.push_back(input.substr(start, l));
        // just copied the last token
        if(end == std::string::npos)
            break;
        // exclude delimiter in next search
        start = end + delim.size();
    }

    return tokens;
}

std::vector<libaan::string_type>
libaan::split2(const std::string &input, const std::string &delim)
{
    std::vector<string_type> tokens;
    if(delim.empty())
        return tokens;

    std::string::size_type start = 0;
    std::string::size_type end;

    for(;;) {
        end = input.find(delim, start);

        // Just copied the last token
        if(end == std::string::npos) {
            tokens.emplace_back(&input.data()[start], input.size() - start);
            break;
        }

        const auto l = end - start;
        if(l == 0) {
            start += delim.size();
            continue;
        }

        tokens.emplace_back(&input.data()[start], l);
        // Exclude the delimiter in the next search
        start = end + delim.size();
    }

    return tokens;
}

std::vector<libaan::string_type> libaan::split(const string_type &input, const string_type &delim)
{
    std::vector<string_type> tokens;
    if(delim.l == 0)
        return tokens;

    const char *start = input.s;
    const char *end = nullptr;

    for(;;) {
        end = std::strstr(start, delim.s);

        // Just copied the last token
        if(end == nullptr) {
            tokens.emplace_back(start, input.s + input.l - start);
            break;
        }

        const auto l = end - start;
        if(l == 0) {
            start += delim.l;
            continue;
        }

        tokens.emplace_back(start, l);
        // Exclude the delimiter in the next search
        start = end + delim.l;
    }

    return tokens;
}

std::vector<libaan::string_type>
libaan::split(const string_type &input, unsigned char delim)
{
    std::vector<string_type> ret;
    const char *ptr = input.s;
    size_t len = input.l;
    do {
        const char *found = reinterpret_cast<const char *>(std::memchr(ptr, delim, len));
        if(!found || !(found + 1)) {
            if(len)
                ret.emplace_back(ptr, len);
            return ret;
        } else {
            const auto tmp = found - ptr;
            if(tmp)
                ret.emplace_back(ptr, tmp);
            ptr = ++found;
            if(len)
                len -= tmp ? tmp + 1 : 1;
        }
    } while(true);
    return ret;
}

/*
string_type find(const string_type &haystack, const std::string &needle)
{
    // equal to std::end(*haystack) ? need to test
    const auto end = haystack.first + haystack.second
    const auto found = std::search(haystack.first, end,
                       std::begin(needle), std::end(needle));
    return found;
}
*/

std::vector<std::size_t>
libaan::search::stl_search_all(const std::string &pattern, const std::string &txt)
{
    std::vector<std::size_t> matches;

    if(pattern.empty())
        return matches;

    std::size_t match = 0;
    while(true) {
        match = txt.find(pattern, match);
        if(match == std::string::npos)
            break;
        matches.push_back(match);
        ++match;
    }
    return matches;
}

libaan::search::sarr_cx11::sarr_cx11(const char *in, size_t l)
 : input(in), input_length(l)
{
     suffixes.resize(input_length);

    std::iota(std::begin(suffixes), std::end(suffixes), 0);

    struct cmp_t
    {
        cmp_t(const char *text) : txt(text) {}
        bool operator()(std::size_t lhs, std::size_t rhs)
        {
            return std::strcmp(txt + lhs, txt + rhs) < 0;
        }
        const char *txt;
    } cmp(input);

    std::sort(std::begin(suffixes), std::end(suffixes), cmp);
}

std::vector<std::size_t> libaan::search::sarr_cx11::search(const std::string &pattern)
{
    std::vector<std::size_t> matches;
    const size_t pattern_length = pattern.length();
    if(pattern_length > input_length || pattern_length == 0)
        return matches;

    // binary search
    std::size_t left = 0;
    std::size_t right = input_length - 1;
    while(left <= right) {
        // See if 'pattern' is prefix of middle suffix in suffix array

        std::size_t mid = left + (right - left) / 2;
        auto res = std::strncmp(pattern.c_str(), input + suffixes[mid],
                                pattern_length);

        if(res == 0) {
            matches.push_back(suffixes[mid]);
            int idx = mid;
            do {
                ++idx;
                if((size_t)idx >= suffixes.size())
                    break;
                res = std::strncmp(pattern.c_str(),
                                   input + suffixes[idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);
            idx = mid;
            // TODO: result is unsorted..
            do {
                if(idx == 0)
                    break;
                --idx;
                res = std::strncmp(pattern.c_str(),
                                   input + suffixes[idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);

            return matches;
        }

        // Move to left half if pattern is alphabtically less than
        // the mid suffix
        if(res < 0)
            right = mid - 1;
        else    // Otherwise move to right half
            left = mid + 1;
    }

    // We reach here if return statement in loop is not executed
    return matches;
}

void libaan::search::sarr_cx11::print()
{
    std::size_t cnt = 0;
    // std::cout << "suffix array for:\n\"" << input << "\"\n";
    for(const auto &suffix_idx : suffixes)
        std::cout << cnt++ << ": \"" << input + suffix_idx << "\"\n";
    std::cout << "\n";
}

libaan::search::sarr_c::sarr_c(const char *txt, int n)
    : suffixes(n), txt(txt), n(n)
{
    struct suffix {
        int index;
        const char *suff;
    };

    // A structure to store suffixes and their indexes
    std::vector<suffix> suffix_array(n);

    // Store suffixes and their indexes in an array of structures.
    // The structure is needed to sort the suffixes alphabatically
    // and maintain their old indexes while sorting
    for(int i = 0; i < n; i++) {
        suffix_array[i].index = i;
        suffix_array[i].suff = (txt + i);
    }

    // Sort the suffixes using the comparison function
    // defined above.
    std::sort(std::begin(suffix_array), std::end(suffix_array),
              [](const suffix &a, const suffix &b) {
                  return strcmp(a.suff, b.suff) < 0 ? 1 : 0; });

    // Store indexes of all sorted suffixes in the suffix array

    for(int i = 0; i < n; i++)
        suffixes[i] = suffix_array[i].index;
}

// TODO: result is unsorted
std::vector<std::size_t> libaan::search::sarr_c::search(const char *pat)
{
    std::vector<std::size_t> matches;

    // needed for strncmp()
    const int pat_len = strlen(pat);
    if(pat_len > n || pat_len == 0)
        return matches;

    // binary search for pat in txt using built suffix array
    // left and right indexes
    int l = 0, r = n - 1;
    while(l <= r) {
        // See if 'pat' is prefix of middle suffix in suffix array
        int mid = l + (r - l) / 2;
        int res = strncmp(pat, txt + suffixes[mid], pat_len);

        // If match found at the middle, print it and return
        if(res == 0) {
            matches.push_back(suffixes[mid]);
            int idx = mid;
            do {
                ++idx;
                if((size_t)idx >= suffixes.size())
                    break;

                res = strncmp(pat, txt + suffixes[idx], pat_len);
                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);
            idx = mid;
            do {
                if(idx == 0)
                    break;
                --idx;
                res = strncmp(pat, txt + suffixes[idx], pat_len);

                if(res != 0)
                    break;
                matches.push_back(suffixes[idx]);
            } while(res == 0);
            return matches;
        }

        // Move to left half if pattern is alphabtically less than
        // the mid suffix
        if(res < 0)
            r = mid - 1;
        else    // Otherwise move to right half
            l = mid + 1;
    }

    // We reach here if return statement in loop is not executed
    return matches;
}

void libaan::search::sarr_c::print()
{
    std::size_t cnt = 0;
    // std::cout << "suffix array for:\n\"" << input << "\"\n";
    for(const auto &suffix_idx : suffixes)
        std::cout << cnt++ << ": \"" << txt + suffix_idx << "\"\n";
    std::cout << "\n";
}

namespace {

void create_source_array(const std::string &in,
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
std::unique_ptr<int[]>
create_suffix_array_buffer(const std::vector<int> &input_text_padded)
{
    const auto n = input_text_padded.size();
    std::unique_ptr<int[]> suffix_array_buffer(new int[n]);

    // dc3 algorithm expects the last 3 elements to be 0.
    suffix_array_buffer[n - 3] = suffix_array_buffer[n - 2]
        = suffix_array_buffer[n - 1] = 0;
    return suffix_array_buffer;
}


/*
Original implementation from whitepaper(see dc3.hh) with some changes, to
improve readibility.
*/

// lexic. order for pairs
inline bool leq(int a1, int a2, int b1, int b2)
{
    return (a1 < b1 || (a1 == b1 && a2 <= b2));
}

// lexic. order for triples
inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3)
{
    return (a1 < b1 || (a1 == b1 && leq(a2, a3, b2, b3)));
}

// O(2*n + K)
// stably sort input[0..n-1] to output[0..n-1] with keys in 0..K from r
static void radixPass(const int *input, int *output, const int *keys,
                      const int len, const int K)
{
    // use vector since we zero initialise it anyway
    std::vector<int> counter_array(K + 1, 0);

    // count occurences
    for(int i = 0; i < len; i++)
        counter_array[keys[input[i]]]++;

    // exclusive prefix sums
    for(int i = 0, sum = 0; i <= K; i++) {
        int t = counter_array[i];
        counter_array[i] = sum;
        sum += t;
    }

    // sort
    for(int i = 0; i < len; i++)
        output[counter_array[keys[input[i]]]++] = input[i];
}

// Find the suffix array SA of source[0..n-1] in keyspace {1..K}^n
// requires:
//   source length = n + 3
//   source[n] = source[n + 1] = source[n + 2] = 0
//   n >= 2
void suffixArray(const int *source, int *SA, const int n, const int K)
{
    const int n0 = (n + 2) / 3;
    const int n1 = (n + 1) / 3;
    const int n2 = n / 3;
    const int n02 = n0 + n2;

    int *s12 = new int[n02 + 3];
    s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;
    int *SA12 = new int[n02 + 3];
    SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;

    int *s0 = new int[n0];
    int *SA0 = new int[n0];

    //******* Step 0: Construct sample ********
    // generate positions of mod 1 and mod  2 suffixes
    // the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
    for(int i = 0, j = 0; i < n + (n0 - n1); i++)
        if(i % 3 != 0)
            s12[j++] = i;

    //******* Step 1: Sort sample suffixes ********
    // lsb radix sort the mod 1 and mod 2 triples
    radixPass(s12, SA12, source + 2, n02, K);
    radixPass(SA12, s12, source + 1, n02, K);
    radixPass(s12, SA12, source, n02, K);

    // find lexicographic names of triples
    int name = 0, c0 = -1, c1 = -1, c2 = -1;
    for(int i = 0; i < n02; i++) {
        if(source[SA12[i]] != c0 || source[SA12[i] + 1] != c1
           || source[SA12[i] + 2] != c2) {
            name++;
            c0 = source[SA12[i]];
            c1 = source[SA12[i] + 1];
            c2 = source[SA12[i] + 2];
        }

        // left half
        if(SA12[i] % 3 == 1)
            s12[SA12[i] / 3] = name;
        // right half
        else
            s12[SA12[i] / 3 + n0] = name;
    }

    // recurse if names are not yet unique
    if(name < n02) {
        suffixArray(s12, SA12, n02, name);
        // store unique names in s12 using the suffix array
        for(int i = 0; i < n02; i++)
            s12[SA12[i]] = i + 1;
    } else    // generate the suffix array of s12 directly
        for(int i = 0; i < n02; i++)
            SA12[s12[i] - 1] = i;

    //******* Step 2: Sort nonsample suffixes ********
    // stably sort the mod 0 suffixes from SA12 by their first character
    for(int i = 0, j = 0; i < n02; i++)
        if(SA12[i] < n0)
            s0[j++] = 3 * SA12[i];
    radixPass(s0, SA0, source, n0, K);

    //******* Step 3: Merge ********
    // merge sorted SA0 suffixes and sorted SA12 suffixes
    for(int p = 0, t = n0 - n1, k = 0; k < n; k++) {
#define get_offset_12() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)

        // pos of current offset 12 suffix
        const int offset_12 = get_offset_12();
        // pos of current offset 0  suffix
        const int offset_0 = SA0[p];

        // is suffix from SA12 smaller?
        if(SA12[t] < n0 ? leq(source[offset_12], s12[SA12[t] + n0],
                              source[offset_0], s12[offset_0 / 3])
                        : leq(source[offset_12], source[offset_12 + 1],
                              s12[SA12[t] - n0 + 1], source[offset_0],
                              source[offset_0 + 1], s12[offset_0 / 3 + n0])) {
            SA[k] = offset_12;
            t++;

            // done --- only SA0 suffixes left
            if(t == n02)
                for(k++; p < n0; p++, k++)
                    SA[k] = SA0[p];
        } else {
            SA[k] = offset_0;
            p++;

            // done --- only SA12 suffixes left
            if(p == n0)
                for(k++; t < n02; t++, k++)
                    SA[k] = get_offset_12();
        }
    }
    delete[] s12;
    delete[] SA12;
    delete[] SA0;
    delete[] s0;
}

}

std::vector<std::size_t> libaan::search::sarr_dc3::search(const std::string &pattern)
{
    std::vector<std::size_t> matches;
    const int pattern_length = pattern.length();
    if((size_t)pattern_length > input_text.length() || pattern_length == 0)
        return matches;

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
            matches.push_back(suffix_array[mid]);
            int idx = mid;
            do {
                ++idx;
                if((size_t)idx >= input_text.size())
                    break;
                res = std::strncmp(pattern.c_str(),
                                   input_text.c_str() + suffix_array[idx],
                                   pattern_length);
                if(res != 0)
                    break;

                matches.push_back(suffix_array[idx]);
            } while(res == 0);

            idx = mid;
            // TODO: result is unsorted..
            do {
                if(idx == 0)
                    break;
                --idx;
                res = std::strncmp(pattern.c_str(),
                                   input_text.c_str() + suffix_array[idx],
                                   pattern_length);
                if(res != 0)
                    break;
                matches.push_back(suffix_array[idx]);
            } while(res == 0);

            return matches;
        }

        // Move to left half if pattern is alphabtically less than
        // the mid suffix
        if(res < 0)
            right = mid - 1;
        else    // Otherwise move to right half
            left = mid + 1;
    }

    // We reach here if return statement in loop is not executed

    return matches;
}

void libaan::search::sarr_dc3::search_and_dump_all(const std::string &pattern)
{
    const auto matches = search(pattern);
    if(matches.empty()) {
        std::cout << "No matches for pattern \"" << pattern << "\"\n";
        return;
    }

    std::cout << matches.size() << " matches for pattern \"" << pattern
              << "\"\n";
}

void libaan::search::sarr_dc3::dump_suffix_array()
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

void libaan::search::sarr_dc3::create()
{
    create_source_array(input_text, input_text_padded, max_key);
    suffix_array = create_suffix_array_buffer(input_text_padded);
    suffixArray(&input_text_padded[0], suffix_array.get(), input_text.length(),
                max_key);
}

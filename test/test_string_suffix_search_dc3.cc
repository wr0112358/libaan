#include "../string_suffix_search_dc3.hh"
#include <iostream>
#include <cmath>

#include <fstream>
#include <string>

inline size_t get_file_length(std::ifstream &fp)
{
    fp.seekg(0, fp.end);
    const int length = fp.tellg();
    if(length == std::fstream::pos_type(-1))
        return 0;
    fp.seekg(0, fp.beg);
    return length;
}

inline bool read_file(const char *file_name, std::string &buff)
{
    std::ifstream fp(file_name);
    const size_t length = get_file_length(fp);
    buff.resize(length);
    char *begin = &*buff.begin();
    fp.read(begin, length);

    return true;
}

#if 0
namespace debug {
inline void assert(bool cond, const std::string &id)
{
    if(!cond)
        std::cout << "\nAssertion violation: " << id << "\n";
}

void print(int *a, int n, const char *comment)
{
    std::cout << comment << ":";
    for(int i = 0; i < n; i++) {
        std::cout << a[i] << " ";
    }
    std::cout << std::endl;
}

bool isPermutation(const int *SA, int n)
{
    bool *seen = new bool[n];
    for(int i = 0; i < n; i++)
        seen[i] = 0;
    for(int i = 0; i < n; i++)
        seen[SA[i]] = 1;
    for(int i = 0; i < n; i++)
        if(!seen[i])
            return 0;
    return 1;
}

bool sleq(const int *s1, const int *s2)
{
    if(s1[0] < s2[0])
        return 1;
    if(s1[0] > s2[0])
        return 0;
    return sleq(s1 + 1, s2 + 1);
}

// is SA a sorted suffix array for s?
bool isSorted(const int *SA, const int *source, int n)
{
    for(int i = 0; i < n - 1; i++) {
        if(!sleq(source + SA[i], source + SA[i + 1]))
            return 0;
    }
    return 1;
}
}
#endif

void do_one(const std::string &input_text)
{

    dc3_api::suffix_array_dc3 dc3_lookup(input_text);
    dc3_lookup.dump_suffix_array();
}

void do_huge(const std::string &input_text)
{

    dc3_api::suffix_array_dc3 dc3_lookup(input_text);
    dc3_lookup.search_and_dump_all("nan");
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    {
// my test:
// wikipedia example(alphabet indices start at 1):
// Für den String "abracadabra" lautet das Suffixarray
// { 11, 8, 1, 4, 6, 9, 2, 5, 7, 10, 3 }
// our output:
// suffix array for "abracadabra":
// 10 7 0 3 5 8 1 4 6 9 2

// my benchmark programm:
// suffix array for: "banana$"       6 5 3 1 0 4 2
// suffix array for: "banana"        5 3 1 0 4 2
// suffix array for: "mississippi$"  11 10 7 4 1 0 9 8 6 3 5 2
// suffix array for: "mississippi"   10 7 4 1 0 9 8 6 3 5 2
// suffix array for: "abracadabra$"  11 10 7 0 3 5 8 1 4 6 9 2
// suffix array for: "abracadabra"   10 7 0 3 5 8 1 4 6 9 2
// suffix array for: "yabbadabbado$" 12 1 6 4 9 3 8 2 7 5 10 11 0
// suffix array for: "yabbadabbado"  1 6 4 9 3 8 2 7 5 10 11 0

// This program outputs:
//  ./dc3_test 
//        suffix array for "banana$":
// 6 5 3 1 0 4 2 
//     suffix array for "banana":
// 5 3 1 0 4 2 
//     suffix array for "mississippi$":
// 11 10 7 4 1 0 9 8 6 3 5 2 
//     suffix array for "mississippi":
// 10 7 4 1 0 9 8 6 3 5 2 
//     suffix array for "abracadabra$":
// 11 10 7 0 3 5 8 1 4 6 9 2 
//     suffix array for "abracadabra":
// 10 7 0 3 5 8 1 4 6 9 2 
//     suffix array for "yabbadabbado$":
// 12 1 6 4 9 3 8 2 7 5 10 11 0 
//     suffix array for "yabbadabbado":
// 1 6 4 9 3 8 2 7 5 10 11 0
//
// after conversion to c++11 constructs it outputs the same:
// $ ./dc3_test 
//     suffix array for "banana$":
// 6 5 3 1 0 4 2 
//     suffix array for "banana":
// 5 3 1 0 4 2 
//     suffix array for "mississippi$":
// 11 10 7 4 1 0 9 8 6 3 5 2 
//     suffix array for "mississippi":
// 10 7 4 1 0 9 8 6 3 5 2 
//     suffix array for "abracadabra$":
// 11 10 7 0 3 5 8 1 4 6 9 2 
//     suffix array for "abracadabra":
// 10 7 0 3 5 8 1 4 6 9 2 
//     suffix array for "yabbadabbado$":
// 12 1 6 4 9 3 8 2 7 5 10 11 0 
//     suffix array for "yabbadabbado":
// 1 6 4 9 3 8 2 7 5 10 11 0

        do_one("banana$");
        do_one("banana");
        do_one("mississippi$");
        do_one("mississippi");
        do_one("abracadabra$");
        do_one("abracadabra");
        do_one("yabbadabbado$");
        do_one("yabbadabbado");

        std::string buff;
        read_file("words.test", buff);
        do_huge(buff);

        return 0;
    }

#if 0
    if(argc < 3) {
        std::cerr << "expecting args: nmax b.\n"
                  << "  nmax: construct strings with length 2 to nmax\n"
                  << "  b: constructed test strings are permutations over\n"
                     "     the keyspace [1..bmax]\n";
        return -1;
    }

    const int nmax = std::atoi(argv[1]);
    const int max_key = std::atoi(argv[2]);
    // try all strings from (1..max_key)^n
    for(int n = 2; n <= nmax; n++) {
        std::cout << "\nn: " << n << "\n";
        const int N = int(std::pow(double(max_key), n) + 0.5);
        int *s = new int[n + 3];
        int *SA = new int[n + 3];

        for(int i = 0; i < n; i++)
            s[i] = SA[i] = 1;
        s[n] = s[n + 1] = s[n + 2] = SA[n] = SA[n + 1] = SA[n + 2] = 0;

        for(int j = 0; j < N; j++) {
            debug::print(s, n, "input s");

            suffixArray(s, SA, n, max_key);

            debug::assert(s[n] == 0, "1");
            debug::assert(s[n + 1] == 0, "2");
            debug::assert(SA[n] == 0, "3");
            debug::assert(SA[n + 1] == 0, "4");
            debug::assert(debug::isPermutation(SA, n), "5");
            debug::assert(debug::isSorted(SA, s, n), "6");

            debug::print(SA, n, "output SA");
            std::cout << "\n";

            // generate next s
            int i;
            for(i = 0; s[i] == max_key; i++)
                s[i] = 1;
            s[i]++;
        }
        delete[] s;
        delete[] SA;
    }
#endif
}

#include "string_suffix_search_dc3_internal.hh"

#include <vector>

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

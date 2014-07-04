/*
Example output:
tokenizer timings(349901 words):
	file-io: 1.22925 ms
	split: 26.8858 ms
	split2: 11.5316 ms
*/
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "libaan/chrono_util.hh"
#include "libaan/string_util.hh"

inline size_t get_file_length(std::ifstream & fp)
{
    fp.seekg(0, fp.end);
    const auto length = fp.tellg();
    if(length == std::fstream::pos_type(-1))
        return 0;
    fp.seekg(0, fp.beg);
    return length;
}

inline bool read_file(const std::string &file_name,
                      std::string &buff)
{
    std::ifstream fp(file_name);
    const size_t length = get_file_length(fp);
    buff.resize(length);
    char *begin = &*buff.begin();
    fp.read(begin, length);

    return true;
}

bool
test_for_equality(const std::vector<std::string> &split_in,
                  const std::vector<std::pair<const char *, size_t>> &split2_in)
{
    if(split_in.size() != split2_in.size()) {
        std::cerr << "test_for_equality failed: number of words differs.\n";
        return false;
    }

    std::size_t error_count = 0;
    const auto size = split_in.size();
    for(std::size_t idx = 0; idx < size; idx++) {
        const std::string &in1 = split_in[idx];
        std::string in2;
        try {
            in2 = std::string(
            split2_in[idx].first, split2_in[idx].first + split2_in[idx].second);
        } catch(const std::length_error &e) {
            std::cout << "CATCHED:\n\tidx = " << idx
                      << "\n\tsplit2_in[idx].second = " << split2_in[idx].second
                      << "\n\tsizeof(split2_in) = " << size
                      << "\n\tcorresponding string split_in[idx] = "
                      << split_in[idx]
                      << "\n";
        };
        if(in1 != in2) {
            error_count++;
            std::cout << "(\"" << in1 << "\", \"" << in2 << "\")\n";
        }
    }

    return error_count == 0;
}

bool split_test_file(const std::string &path)
{
    std::string file_buffer;
    libaan::util::time_me_ns timer;
    read_file(path, file_buffer);
    const double io_time = timer.duration();
    const std::string DELIM = {' '};
    timer.restart();
    const auto tokens = libaan::util::split(file_buffer, DELIM);
    const double split_time = timer.duration();
    if(tokens.empty())
        return false;

    timer.restart();
    const auto tokens2 = libaan::util::split2(file_buffer, DELIM);
    const double split2_time = timer.duration();
    if(tokens2.empty())
        return false;

    std::cout << "tokenizer timings(" << tokens2.size() << " words):\n"
              << "\tfile-io: " << io_time / (1000.0 * 1000.0) << " ms\n"
              << "\tsplit: " << split_time / (1000.0 * 1000.0) << " ms\n"
              << "\tsplit2: " << split2_time / (1000.0 * 1000.0) << " ms\n\n";


    if(!test_for_equality(tokens, tokens2)) {
        std::cout << "\nBUGS here.\n\n";
        return false;
    }

    return true;
}

int main(int, char *[])
{
    split_test_file("words.test");
    exit(EXIT_SUCCESS);
}

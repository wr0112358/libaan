#include "debug.hh"

#include <cstring>
#include <iostream>
#include "file.hh"

void libaan::do_ls(const std::string &path, std::ostream &out)
{
    auto const f = [&out](const std::string &, const struct dirent *p) {
        out << "dirent = { "
        << ".d_ino = " << std::to_string(p->d_ino)
        << ", .d_off = " << std::to_string(p->d_off)
        << ", .d_reclen = " << std::to_string(p->d_reclen)
        << ".d_name = \"" << p->d_name << "\" }\n";
    };
    out << path << "\n";
    libaan::readdir(path, f);
}

namespace {
int ascii2bin(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  else if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  else if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  else
      return -1;
}
}

// Src must have an even number of [0-9a-f] characters.
// In the uneven case it returns false and converted input.
std::pair<bool, std::vector<unsigned char> > libaan::hex2bin(const char *src, size_t size_hint)
{
    std::vector<unsigned char> target;
    if(size_hint) {
        target.reserve(size_hint / 2);
        size_hint = 0;
    }

    while(*src && src[1]) {
        const auto a = ascii2bin(*src) * 16;
        const auto b = ascii2bin(src[1]);
        if(a < 0 || b < 0)
            return std::make_pair(false, decltype(target)());
        target.push_back(a + b);
        src += 2;
        size_hint++;
    }

    target.resize(size_hint);
    return std::make_pair(!*src, target);
}

std::string libaan::bin2hex(const std::vector<unsigned char> &v)
{
    std::string s(v.size() * 2, '\0');
    for(size_t i = 0; i < v.size(); i++) {
//        printf("%02x ", (int)v[i]);
        snprintf(&s[i * 2], 3, "%02x", (int)v[i]);
//        std::cout << s << "\n";
    }
    return s;
}

void libaan::strip(std::string &str)
{
    auto i = str.size();
    for(; i-- > 0 ; )
        if(str[i] != '\0')
            break;
    str.resize(i + 1);
}

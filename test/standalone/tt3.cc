#include <fstream>
#include <iostream>

void print(const char *prefix, const std::ofstream &of)
{
    std::cout << prefix
              << ": bad=" << of.bad()
              << ", fail=" << of.fail()
              << ", eof=" << of.eof()
              << ", good=" << of.good()
              << (of.bad() || of.fail()
                  ? std::string(", errno=\"") + std::string(sys_errlist[errno])
                  : std::string(""))
              << "\n";
}

#include "libaan/file.hh"

int main()
{
    std::ofstream o1;
    print("empty constructor", o1);
    print("empty constructor again", o1);
    o1.clear(std::ios_base::badbit);
    print("empty constructor + clear(badbit)", o1);
    std::cout << "\n";

    std::ofstream o2("/etc/motd");
    print("readonly file", o2);
    print("readonly file again", o2);
    std::cout << "\n";

    std::ofstream o3("/dev/null");
    print("writeonly file", o3);
    print("writeonly file again", o3);
    std::cout << "\n";

    std::ofstream o4;
    libaan::temp_file(o4);
    print("temp_file()", o4);
    std::cout << "\n";

    std::ofstream o5;
    libaan::temp_file(o5, "xx");
    print("temp_file(xx, xx)", o5);
}

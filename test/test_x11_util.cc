#include <chrono>
#include <iostream>
#include <thread>

#include "../x11_util.hh"

int main(int argc, char *argv[])
{
    using namespace libaan::util::x11;
    if(argc != 2) {
        std::cout << "USAGE: " << argv[0] << " \"string to put in clipboard\"\n";
        exit(EXIT_FAILURE);
    }

    if(!add_to_clipboard(argv[1], std::chrono::milliseconds(3000))) {
        std::cout << "Failure.\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Successfully received clipboard request!\n";
    exit(EXIT_SUCCESS);
}

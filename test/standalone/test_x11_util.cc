#include <chrono>
#include <iostream>
#include <thread>

#include "libaan/x11.hh"

int main(int argc, char *argv[])
{
    if(argc != 2) {
        std::cout << "USAGE: " << argv[0] << " \"string to put in clipboard\"\n";
        exit(EXIT_FAILURE);
    }

    if(!libaan::add_to_clipboard(argv[1], std::chrono::milliseconds(10000))) {
        std::cout << "Failure.\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Successfully received clipboard request!\n";
    exit(EXIT_SUCCESS);
}

#include "libaan/terminal.hh"
#include <iostream>
#include <chrono>
#include <thread>

void test3()
{
    libaan::rawmode tty_raw;
    const std::chrono::milliseconds dura(2000);
    while(true) {
        bool readsth = false;
        while (tty_raw.kbhit()) {
            const auto in = tty_raw.getch();
            char c = static_cast<char>(in);
            std::cout << "\tyes: " << c << "\t" << in << "\n";
            if(c == 'q')
                return;
            readsth = true;
        }
        if(!readsth)
            std::cout << "[ ]\n";
        std::cout << "sleep 2000ms ->\n";
        std::this_thread::sleep_for(dura);
        std::cout << "sleep 2000ms <-\n";
    }

}

void test4()
{
    const std::chrono::milliseconds dura(2000);
    while(true) {
        libaan::rawmode tty_raw;
        bool readsth = false;
        while (tty_raw.kbhit()) {
            const auto in = tty_raw.getch();
            char c = static_cast<char>(in);
            std::cout << "\tyes: " << c << "\t" << in << "\n";
            if(c == 'q')
                return;
            if(c == '\n' || c == '\r')
                std::cout << "\n";
            readsth = true;
        }
        if(!readsth)
            std::cout << "[ ]\n";
        std::cout << "sleep 2000ms ->\n";
        std::this_thread::sleep_for(dura);
        std::cout << "sleep 2000ms <-\n";
    }

}

int main()
{
    std::cout << "Press 'q' to exit\n";
    test4();
    return 0;
}

#include "../terminal_util.hh"
#include <iostream>
#include <chrono>
#include <thread>

// usage example 1. Messes up signals.
void test1()
{
    libaan::util::rawmode tty_raw;
    const std::chrono::milliseconds dura(2000);
    while(true) {
        std::cout << "1. kbhit?\n";

        if (tty_raw.kbhit()) {
            char c = static_cast<char>(tty_raw.getch());
            std::cout << "\tyes: " << c << "\n";
            if(c == 'q')
                return;
        } else
            std::cout << "\tno\n";
        std::cout << "2. Before sleep(2000ms)\n";
        std::this_thread::sleep_for(dura);
        std::cout << "3. After sleep(2000ms)\n";
    }

}

void test2()
{
    const std::chrono::milliseconds dura(2000);
    while(true) {
        std::cout << "1. kbhit?\n";
        {
            libaan::util::rawmode tty_raw;
            if (tty_raw.kbhit()) {
                char c = static_cast<char>(tty_raw.getch());
                std::cout << "\tyes: " << c << "\n";
                if(c == 'q')
                    return;
            } else
                std::cout << "\tno\n";
        }
        std::cout << "2. Before sleep(2000ms)\n";
        std::this_thread::sleep_for(dura);
        std::cout << "3. After sleep(2000ms)\n";
    }

}

// loop over all inputs
void test3()
{
    const std::chrono::milliseconds dura(2000);
    while(true) {
        std::cout << "1. kbhit?\n";
        {
            libaan::util::rawmode tty_raw;
            bool readsth = false;
            while (tty_raw.kbhit()) {
                char c = static_cast<char>(tty_raw.getch());
                std::cout << "\tyes: " << c << "\n";
                if(c == 'q')
                    return;
                readsth = true;
            }
            if(!readsth)
                std::cout << "\tno\n";
        }
        std::cout << "2. Before sleep(2000ms)\n";
        std::this_thread::sleep_for(dura);
        std::cout << "3. After sleep(2000ms)\n";
    }

}



int main()
{
    test3();
    return 0;
}
#ifndef _LIBAAN_TERMINAL_HH_
#define _LIBAAN_TERMINAL_HH_

#include <cstdio>
#include <cstdlib>

#include <memory>
#include <tuple>

#include <cassert>


#ifdef NO_GOOD

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;   /* unused */
    unsigned short ws_ypixel;   /* unused */
};

#else

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>    // STDOUT_FILENO

#endif

namespace libaan {

void alternate_screen_on();
void alternate_screen_off();

std::pair<bool, int> do_poll_on_stdin_plus1(int fd, int timeout_secs = 5);
std::pair<bool, char> blocking_read_one(size_t ms = 5000);

struct io {
    io();
    ~io();

private:

    bool save_termios();
    bool restore_termios();

    struct termios oldtermios;
};

struct terminal_size {
    terminal_size()
    {
        size.ws_row = size.ws_col = 0;
        sigwinch_fd = -1;
        winsize_signal_handler();
        update_terminal_size();
    }

    ~terminal_size()
    {
        close(sigwinch_fd);
    }

    bool size_changed();
    size_t rows() const { return size.ws_row; }
    size_t cols() const { return size.ws_col; }

private:
    bool update_terminal_size();
    bool winsize_signal_handler();

    struct winsize size;
    int sigwinch_fd;
};

class terminal
{
public:
    terminal() noexcept;

    const winsize &get_size() const noexcept;
    bool size_changed() noexcept;
    // maybe integrate BA_tty_functions

private:
    bool update_terminal_size() noexcept;

#ifndef NO_GOOD
    bool winsize_signal_handler() noexcept;
#endif

    struct winsize size;
    int sigwinch_fd;
};

class rawmode {
 public:
    rawmode();
    ~rawmode();

    // Is a character available from stdin?
    // Does the following:
    // - try to read one character without blocking
    // - if succesfull it stores the read character for later non-blocking reads
    //   via getch and return true
    // - else return false
    bool kbhit();
    // Return one character without blocking, if a prepended call to kbhit
    // returned true. Else do a blocking read.
    int getch();

    operator bool() const { return state; }

 private:
    bool save_termios();
    bool restore_termios();

#ifndef NO_GOOD
 private:
    struct termios oldtermios;
#endif
    bool state;
};

}

#endif

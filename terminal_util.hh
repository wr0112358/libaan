/*
Copyright (C) 2014 Reiter Wolfgang wr0112358@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _LIBAAN_TERMINAL_UTIL_HH_
#define _LIBAAN_TERMINAL_UTIL_HH_

#include <cstdio>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>    // STDOUT_FILENO

#include "error_util.hh"

namespace libaan {
namespace util
{
class terminal
{
    public:
    terminal() noexcept;

    void alternate_screen_on() noexcept;
    void alternate_screen_off() noexcept;
    const winsize &get_size() const noexcept;
    bool size_changed() noexcept;
    // maybe integrate BA_tty_functions
    const std::system_error &get_error() const noexcept;

    private:
    bool update_terminal_size() noexcept;
    bool winsize_signal_handler() noexcept;
    std::system_error error;
    struct winsize size;
    int sigwinch_fd;
};


/* http://stackoverflow.com/questions/1513734/problem-with-kbhitand-getch-for-linux */
/* http://linux-sxs.org/programming/kbhit.html */

class rawmode {
 public:
    rawmode(int fileno = STDIN_FILENO);
    ~rawmode();

    bool kbhit();
    int getch();

 private:
    bool tty_raw(bool on);
    bool tty_reset();

 private:
    struct termios oldtermios;
    struct termios newtermios;
    int fd;
    int peek;
};

rawmode::rawmode(int fileno)
        : fd(fileno), peek(-1)
{
    tcgetattr(0,&oldtermios);
    newtermios = oldtermios;
    newtermios.c_lflag &= static_cast<decltype(newtermios.c_lflag)>(~ICANON);
    newtermios.c_lflag &= static_cast<decltype(newtermios.c_lflag)>(~ECHO);
    newtermios.c_lflag &= static_cast<decltype(newtermios.c_lflag)>(~ISIG);
    newtermios.c_cc[VMIN] = 1;
    newtermios.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newtermios);
}

rawmode::~rawmode()
{
    tcsetattr(0, TCSANOW, &oldtermios);
}

bool rawmode::kbhit()
{
    unsigned char ch;
    ssize_t nread;

    if(peek != -1)
        return false;
    newtermios.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &newtermios);
    nread = read(0, &ch, 1);
    newtermios.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &newtermios);

    if(nread == 1) {
        peek = ch;
        return true;
    }

    return false;
}

int rawmode::getch()
{
    char ch;

    if (peek != -1) {
        ch = static_cast<char>(peek);
        peek = -1;
    } else
        read(0, &ch, 1);

    return ch;
}

inline bool rawmode::tty_raw(bool on)
{
    if(!on)
        return tty_reset();

    /* Set terminal mode as follows:
       Noncanonical mode - turn off ICANON.
       Turn off signal-generation (ISIG)
        including BREAK character (BRKINT).
       Turn off any possible preprocessing of input (IEXTEN).
       Turn ECHO mode off.
       Disable CR-to-NL mapping on input.
       Disable input parity detection (INPCK).
       Disable stripping of eighth bit on input (ISTRIP).
       Disable flow control (IXON).
       Use eight bit characters (CS8).
       Disable parity checking (PARENB).
       Disable any implementation-dependent output processing (OPOST).
       One byte at a time input (MIN=1, TIME=0).
    */

    if(tcgetattr(fd, &oldtermios) < 0)
        return false;
    newtermios = oldtermios;

    newtermios.c_lflag &= static_cast<decltype(newtermios.c_lflag)>(~(ECHO | ICANON | IEXTEN | ISIG));
    /* OK, why IEXTEN? If IEXTEN is on, the DISCARD character
       is recognized and is not passed to the process. This 
       character causes output to be suspended until another
       DISCARD is received. The DSUSP character for job control,
       the LNEXT character that removes any special meaning of
       the following character, the REPRINT character, and some
       others are also in this category.
    */

    newtermios.c_iflag &= static_cast<decltype(newtermios.c_iflag)>(
        ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    /* If an input character arrives with the wrong parity, then INPCK
       is checked. If this flag is set, then IGNPAR is checked
       to see if input bytes with parity errors should be ignored.
       If it shouldn't be ignored, then PARMRK determines what
       character sequence the process will actually see.

       When we turn off IXON, the start and stop characters can be read.
    */

    newtermios.c_cflag &= static_cast<decltype(newtermios.c_cflag)>(~(CSIZE | PARENB));
    /* CSIZE is a mask that determines the number of bits per byte.
       PARENB enables parity checking on input and parity generation
       on output.
    */

    newtermios.c_cflag |= CS8;
    /* Set 8 bits per character. */

    newtermios.c_oflag &= static_cast<decltype(newtermios.c_oflag)>(~(OPOST));
    /* This includes things like expanding tabs to spaces. */

    newtermios.c_cc[VMIN] = 1;
    newtermios.c_cc[VTIME] = 0;

    /* You tell me why TCSAFLUSH. */
    if(tcsetattr(fd, TCSAFLUSH, &newtermios) < 0)
        return false;
    return true;
}

inline bool rawmode::tty_reset()
{
    if(tcsetattr(fd, TCSAFLUSH, &oldtermios) < 0)
        return false;

    return true;
}

}
}

libaan::util::terminal::terminal() noexcept
{
    size.ws_row = size.ws_col = 0;
    sigwinch_fd = -1;
    winsize_signal_handler();
    update_terminal_size();
}

void libaan::util::terminal::alternate_screen_on() noexcept
{
    printf("\033[?1049h\033[H");
}

void libaan::util::terminal::alternate_screen_off() noexcept
{
    printf("\033[?1049l");
}

const winsize &libaan::util::terminal::get_size() const noexcept { return size; }

bool libaan::util::terminal::size_changed() noexcept
{
    struct pollfd pfd[1];
    pfd[0].fd = sigwinch_fd;
    pfd[0].events = POLLIN | POLLERR | POLLHUP;
    int poll_return;
    error = check2(poll, poll_return, pfd, 1, 1);
    if (error.code())
        return false;

    if (!poll_return) {
        return false;
    }

    struct signalfd_siginfo info;
    ssize_t bytes;
    error = check2(read, bytes, sigwinch_fd, &info, sizeof(info));
    if (error.code())
        return false;

    if (info.ssi_signo != SIGWINCH) {
        puts("if(info.ssi_signo != SIGWINCH)");
        // TODO: set error to custom message
        // TODO: is error possible?
        return false;
    }
    printf("SIGWINCH from user %u. Updating terminal size...\n", info.ssi_uid);

    return update_terminal_size();
}

const std::system_error &libaan::util::terminal::get_error() const noexcept
{
    return error;
}

bool libaan::util::terminal::update_terminal_size() noexcept
{
    error = check(ioctl, STDOUT_FILENO, TIOCGWINSZ, &size);
    if (error.code())
        return false;

    printf("lines %d\n", size.ws_row);
    printf("columns %d\n", size.ws_col);

    return true;
}

bool libaan::util::terminal::winsize_signal_handler() noexcept
{
    sigset_t sigset;
    error = check(sigemptyset, &sigset);
    if (error.code())
        return false;

    error = check(sigaddset, &sigset, SIGWINCH);
    if (error.code())
        return false;

    error = check(sigprocmask, SIG_BLOCK, &sigset, nullptr);
    if (error.code())
        return false;

    error = check2(signalfd, sigwinch_fd, -1, &sigset, 0);
    if (error.code())
        return false;

    return true;
}

#endif

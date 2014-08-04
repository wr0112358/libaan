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
#include <cstdlib>
#ifdef NO_GOOD
#include <conio.h>

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;   /* unused */
    unsigned short ws_ypixel;   /* unused */
};

#else
#include <poll.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <fcntl.h>
//#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>    // STDOUT_FILENO

#endif

#include "error_util.hh"

namespace libaan {
namespace util
{
class terminal
{
    public:
    terminal() noexcept;

    static void alternate_screen_on() noexcept;
    static void alternate_screen_off() noexcept;
    const winsize &get_size() const noexcept;
    bool size_changed() noexcept;
    // maybe integrate BA_tty_functions
    const std::system_error &get_error() const noexcept;

    private:
    bool update_terminal_size() noexcept;

#ifndef NO_GOOD
    bool winsize_signal_handler() noexcept;
#endif

    std::system_error error;
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
    bool state;
#endif
};

rawmode::rawmode()
{
    state = false;

#ifndef NO_GOOD
    if(!save_termios())
        return;
    struct termios tty;
    if(tcgetattr(STDIN_FILENO, &tty) < 0) {
        fprintf(stderr, "\nERROR:  unable to get tty attributes for %s\n",
                ctermid(NULL));
        return;
    }

    // no input flags needed
    tty.c_iflag = 0x0000;
    tty.c_oflag = ONLCR | OPOST;
    tty.c_cflag = CREAD | CSIZE;
    // noncanonical mode, since no ICANON
    tty.c_lflag = IEXTEN;
    tty.c_cc[VERASE] = 0x7F;

    if(tcsetattr(STDIN_FILENO, TCSANOW, &tty) < 0) {
        fprintf(stderr, "\nERROR:  unable to set tty attributes for %s\n",
                ctermid(NULL));
        return;
    }

    state = true;
#endif
}

rawmode::~rawmode()
{
#ifndef NO_GOOD
    restore_termios();
#endif
}

bool rawmode::save_termios()
{
#ifdef NO_GOOD
    return false;
#else
    int fd = open(ctermid(NULL), O_RDONLY);
    if(fd == -1)
        return false;

    if(tcgetattr(fd, &oldtermios) == -1) {
        close(fd);
        return false;
    }
    close(fd);

    return true;
#endif

}

bool rawmode::restore_termios()
{
#ifdef NO_GOOD
    return false;
#else
    int fd = open(ctermid(NULL), O_RDWR);
    if(fd == -1)
        return false;
    if(tcsetattr(fd, TCSANOW, &oldtermios) == -1)
        return false;
    close(fd);

    return true;
#endif
}

bool rawmode::kbhit()
{
#ifndef NO_GOOD
    struct timeval tvptr;
    fd_set rset;
    int status = -1;

    // no blocking
    tvptr.tv_sec = 0;
    tvptr.tv_usec = 0;
    FD_ZERO(&rset);

    FD_SET(STDIN_FILENO, &rset);
    status = select(STDIN_FILENO + 1, &rset, NULL, NULL, &tvptr);
    if(status == -1) {
        fprintf(stderr, "\nERROR:  dataready():  select() returned -1\n");
        return false;
    }

    if(!FD_ISSET(STDIN_FILENO, &rset))
        return false;

    return true;
#else
    return kbhit();
#endif
}

int rawmode::getch()
{
#ifndef NO_GOOD
    char ch;
    if(read(STDIN_FILENO, &ch, 1) != 1) {
        state = false;
        puts("read() != 1");
        return 0;
    }
    return ch;
#else
    return getch();
#endif
}

}
}

libaan::util::terminal::terminal() noexcept
{
#ifndef NO_GOOD
    size.ws_row = size.ws_col = 0;
    sigwinch_fd = -1;
    winsize_signal_handler();
    update_terminal_size();
#endif
}

void libaan::util::terminal::alternate_screen_on() noexcept
{
#ifndef NO_GOOD
    printf("\033[?1049h\033[H");
#else
    system("cls");
#endif
}

void libaan::util::terminal::alternate_screen_off() noexcept
{
#ifndef NO_GOOD
    printf("\033[?1049l");
#endif
}

const winsize &libaan::util::terminal::get_size() const noexcept
{
    return size;
}

bool libaan::util::terminal::size_changed() noexcept
{
#ifndef NO_GOOD
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
#else
    return false;
#endif
}

const std::system_error &libaan::util::terminal::get_error() const noexcept
{
    return error;
}

bool libaan::util::terminal::update_terminal_size() noexcept
{
#ifndef NO_GOOD
    error = check(ioctl, STDOUT_FILENO, TIOCGWINSZ, &size);
    if (error.code())
        return false;

    printf("lines %d\n", size.ws_row);
    printf("columns %d\n", size.ws_col);

    return true;
#else
    return false;
#endif
}


#ifndef NO_GOOD
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

#endif

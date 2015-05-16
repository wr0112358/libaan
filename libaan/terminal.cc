#include "terminal.hh"

#ifdef NO_GOOD

#include <conio.h>

#else

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif

void libaan::alternate_screen_on()
{
#ifndef NO_GOOD
    printf("\033[?1049h\033[H");
#else
    system("cls");
#endif

    //static const std::string alt = "\033[?1049h\033[H";
    //write(STDOUT_FILENO, alt.c_str(), alt.size());
}

void libaan::alternate_screen_off()
{
#ifndef NO_GOOD
    printf("\033[?1049l");
#endif
    //static const std::string alt = "\033[?1049l";
    //write(STDOUT_FILENO, alt.c_str(), alt.size());
}

libaan::io::io()
{
    errno = 0;
    if(!save_termios()) {
        perror("save_termios");
        return;
    }
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
    perror("tcsetattr");
}

libaan::io::~io()
{
    if(!restore_termios())
        perror("restore_termios");
}

bool libaan::io::save_termios()
{
    int fd = open(ctermid(NULL), O_RDONLY);
    if(fd == -1)
        return false;

    if(tcgetattr(fd, &oldtermios) == -1) {
        close(fd);
        return false;
    }
    close(fd);

    return true;
}

bool libaan::io::restore_termios()
{
    int fd = open(ctermid(NULL), O_RDWR);
    if(fd == -1)
        return false;
    if(tcsetattr(fd, TCSANOW, &oldtermios) == -1)
        return false;
    close(fd);

    return true;
}

bool libaan::terminal_size::size_changed()
{
    struct pollfd pfd[1];
    pfd[0].fd = sigwinch_fd;
    pfd[0].events = POLLIN | POLLERR | POLLHUP;
    int err = poll(pfd, 1, 1);
    if (err == -1)
        return false;
    if (err == 0)
        return false;

    struct signalfd_siginfo info;
    ssize_t bytes = read(sigwinch_fd, &info, sizeof(info));
    if (bytes == -1)
        return false;

    if (info.ssi_signo != SIGWINCH) {
        puts("if(info.ssi_signo != SIGWINCH)");
        return false;
    }
    printf("SIGWINCH from user %u. Updating terminal size...\n", info.ssi_uid);

    return update_terminal_size();
}

bool libaan::terminal_size::update_terminal_size()
{
    int err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    if (err == -1)
        return false;

    printf("lines %d\n", size.ws_row);
    printf("columns %d\n", size.ws_col);

    return true;
}

bool libaan::terminal_size::winsize_signal_handler()
{
    sigset_t sigset;
    int err = sigemptyset(&sigset);
    if (err == -1)
        return false;

    err = sigaddset(&sigset, SIGWINCH);
    if (err == -1)
        return false;

    err = sigprocmask(SIG_BLOCK, &sigset, nullptr);
    if (err == -1)
        return false;

    sigwinch_fd = signalfd(-1, &sigset, 0);
    if (err == -1)
        return false;

    return true;
}

std::pair<bool, char> libaan::blocking_read_one(size_t ms)
{
    struct timeval tvptr;
    fd_set rset;

    tvptr.tv_sec = (ms >= 1000) ? ms / 1000 : 0;
    tvptr.tv_usec = (ms % 1000) * 1000;

    FD_ZERO(&rset);
    FD_SET(STDIN_FILENO, &rset);

    const auto ret = select(STDIN_FILENO + 1, &rset, NULL, NULL, &tvptr);
    if(ret == -1) {
        fprintf(stderr, "\nERROR:  dataready():  select() returned -1\n");
        return std::make_pair(false, 0);
    } else if(ret == 0) {
        return std::make_pair(false, 0);
    }

    if(!FD_ISSET(STDIN_FILENO, &rset))
        return std::make_pair(false, 0);
    char ch;
    if(read(STDIN_FILENO, &ch, 1) != 1) {
        //puts("read() != 1");
        return std::make_pair(false, 0);
    }

    //printf(" read: %d\n", (int)ch);
    return std::make_pair(true, ch);
}

std::pair<bool, int> libaan::do_poll_on_stdin_plus1(int fd, int timeout_secs)
{
    struct timeval tvptr;
    fd_set rset;
    tvptr.tv_sec = timeout_secs;
    tvptr.tv_usec = 0;
    FD_ZERO(&rset);
    FD_SET(STDIN_FILENO, &rset);
    FD_SET(fd, &rset);
    const int err = select(std::max(STDIN_FILENO, fd) + 1, &rset, NULL, NULL, &tvptr);
    if(err == -1) {
        perror("select");
        return std::make_pair(false, -1);
    } else if(err == 0) {
//        std::cerr << "  ..select() timeout\n";
        return std::make_pair(true, -1);
    }

    if(FD_ISSET(STDIN_FILENO, &rset)) {
        //std::cerr << "select() returned stdin(fd 0) ready for io!\n";
        return std::make_pair(true, STDIN_FILENO);
    } else if(FD_ISSET(fd, &rset)) {
        //std::cerr << "select() returned mqueue ready for io!\n";
        return std::make_pair(true, fd);
    } else {
//        std::cerr << "  ..ERROR: select() returned unknown\n";
        assert(false);
        return std::make_pair(false, -1);
    }
}

libaan::rawmode::rawmode()
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

libaan::rawmode::~rawmode()
{
#ifndef NO_GOOD
    restore_termios();
#endif
}

bool libaan::rawmode::save_termios()
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

bool libaan::rawmode::restore_termios()
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

bool libaan::rawmode::kbhit()
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

int libaan::rawmode::getch()
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

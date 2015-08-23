#include "fd.hh"

#include <unistd.h>

#include <cerrno>

// Read from fd in the buffer buff with maximum length len.
int libaan::readall(int fd, void *buff, size_t len)
{
    int n;
    size_t nread = 0;

    do {
        n = read(fd, &((char *)buff)[nread], len-nread);
        if(n == -1) {
            if(errno == EINTR)
                continue;
            else
                return -1;
        }
        if(n == 0)
            return nread;
        nread += n;
    } while(nread < len);

    return nread;
}

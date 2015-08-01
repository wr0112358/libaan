#ifndef _LIBAAN_PROCESS_HH_
#define _LIBAAN_PROCESS_HH_

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <utility>

namespace libaan {

/* Example:
    for(int i = 0; i < 10; i++) {
        const auto r = libaan::sub_do([](const char *s, int ii) {
                printf("child[%d]: arg=%d, \"%s\"\n", getpid(), ii, s);
                return ii % 2 == 0; },
            "lambda_in_child", i + 100);
        if(!r)
            perror("lambda_in_child");
        printf("lambda_in_child -> %s\n", r ? "true" : "false");
    }
*/
template <typename ...arg_types, typename lambda_type>
bool sub_do(lambda_type&& lambda, arg_types&& ... args)
{
    errno = 0;
    const auto pid = fork();
    switch(pid) {
    case 0:
        exit(lambda(std::forward<arg_types>(args)...)
             ? EXIT_SUCCESS : EXIT_FAILURE);
    case -1:
        return false;
    default:
        int status;
        if(waitpid(pid,&status,0) == -1)
            return false;
        return WIFEXITED(status) ? WEXITSTATUS(status) == EXIT_SUCCESS : false;
    }
}

}

#endif

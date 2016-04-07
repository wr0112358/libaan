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
template <typename ...child_arg_types, typename child_lambda_type, typename parent_lambda_type>
bool sub_do(parent_lambda_type&& parent_lambda,
            child_lambda_type&& child_lambda,
            child_arg_types&& ... child_args)
{
    errno = 0;
    const auto pid = fork();
    //printf("sub_do fork -> %d\n", pid);
    switch(pid) {
    case 0:
        //printf("XX: start child_lambda\n");
        exit(child_lambda(std::forward<child_arg_types>(child_args)...)
             ? EXIT_SUCCESS : EXIT_FAILURE);
        //printf("XX: DONE child_lambda\n");
        assert(false);
    case -1:
        //printf("XX: ERROR\n");
        return false;
    default:
        //printf("XX: START parent_lambda %d for child %d\n", getpid(), pid);
        return parent_lambda(pid);
    }
}

std::pair<bool, bool> try_reap(pid_t pid)
{
    while(true) {
        int status;
        const auto p = waitpid(pid, &status, WNOHANG);
        // if(p == -1 && errno == EINTR) continue; //not for WNOHANG
        //printf("waitpid(%d) -> %d\n", pid, p);
        if(p == 0)
            return std::make_pair(false, false);
        else if(p == -1)
            return std::make_pair(true, false);
        else if(p == pid)
            return std::make_pair(true, WIFEXITED(status) ? WEXITSTATUS(status) == EXIT_SUCCESS : false);
    }
    return std::make_pair(false, false);
}

}

#endif

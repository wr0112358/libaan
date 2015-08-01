
/*
static void line_wise_diff(const std::string & l, const std::string & r, std::string & result)
{
    result = "";
    std::istringstream lss(l);
    std::istringstream rss(r);
    std::string lline, rline;
    size_t line_nr = 0;
    while(true) {
        const bool lok = std::getline(lss, lline);
        const bool rok = std::getline(rss, rline);
        if(!lok && !rok)
            break;
        if(!lok)
            result += std::to_string(line_nr) + ": > " + rline + "\n";
        else if(!rok)
            result += std::to_string(line_nr) + ": < " + lline + "\n";
        else if(lline != rline){
            result = result + std::to_string(line_nr) + ": " + lline + "\n";
            result += "!=\n";
            result = result + std::to_string(line_nr) + ": " + rline + "\n";
        }
        line_nr++;
    }
}
        std::string diff;
        line_wise_diff(src_buff, dst_decrypted, diff);
        std::cout << diff;
*/

/*
#include "process.hh"

#include <cstdio>

// fork to function
int sub_do(void(*fn)(int r),int r) {
  int i;
  if ((i=fork())==0) {
    fn(r);
    exit(0);
  } else if (i==-1) {
      perror("fork");
      exit(EXIT_FAILURE);
  }
  waitpid(i,&i,0);
  return i;
}

void f(int r) { printf("child[%d]: arg=%d\n", getpid(), r); }

int main()
{
    for(int i = 0; i < 10; i++) {
        const auto r = libaan::sub_do([](const char *s, int ii) {
                printf("child[%d]: arg=%d, \"%s\"\n", getpid(), ii, s);
                return ii % 2 == 0; },
            "lambda_in_child", i + 100);
        if(!r)
            perror("lambda_in_child");
        printf("lambda_in_child -> %s\n", r ? "true" : "false");
    }
    exit(EXIT_SUCCESS);
}

*/

#include <iostream>

bool sleq(const int *s1, const int *s2)
{
    std::cout << "sleq(" << s1[0] << ", " << s2[0] << ")\n";
    if(s1[0] < s2[0])
        return true;
    if(s1[0] > s2[0])
        return false;
    return sleq(s1 + 1, s2 + 1);
}

int main()
{
    const int a[] = { 1, 2, 3, 4, 0, 0, 0 };
//    const int b[] = { 1, 2, 3, 4, 0, 0, 0 };
    const int c[] = { 1, 2, 3, 0, 0, 0 };
//    std::cout << std::boolalpha << sleq(a, a) << "\n";
    std::cout << std::boolalpha << sleq(a, c) << "\n";
    std::cout << std::boolalpha << sleq(c, a) << "\n";
}

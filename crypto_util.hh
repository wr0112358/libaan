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

#ifndef _LIBAAN_CRYPTO_UTIL_HH_
#define _LIBAAN_CRYPTO_UTIL_HH_

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <string>

namespace libaan
{
namespace crypto
{
namespace util
{
/* Usage:
   {
   const password_from_stdin pw(6);
   std::cout << "password_from_stdin: ";
   if(!pw)
       std:: cout << "\"" << pw2.password << "\"\n";
   else {
       std::cout << "failed.\n";
       return false;
   }
   }
*/



// since getpass(3) is deprecated, maybe use something self written.
// or read_passphrase(...) from this truecrypt fork looks good at first glance.
// https://github.com/bwalex/tc-play/blob/39125738741a43ee5888b862ab998854ba5ccf3b/io.c#L393
//
// or openbsd/signify/readpasphrase.c

#ifdef NO_GOOD
char *getpass(const char *prompt)
{
    static char buffer[255];
    memset(buffer, 0, 255);

    fprintf(stderr, "%s", prompt);
    // Disable character echoing and line buffering
    HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if(!GetConsoleMode(hstdin, &mode))
        return 0;
    if(hstdin == INVALID_HANDLE_VALUE || !(SetConsoleMode(hstdin, 0)))
        return 0; // Failed to disable buffering
        
    int i = 0;
    int c = 0;
    for (; (c = getchar()) != '\n' && c != EOF; ++i) {
        if (i > 255)
            return 0;
        buffer[i] = c;
    }
    if(buffer[0])
        puts("***");
    if (!SetConsoleMode(hstdin, mode))
        return 0;

    return buffer;
}
#endif

struct password_from_stdin
{
    password_from_stdin(size_t pw_minlength = 1,
                        const std::string &prompt = "Enter password: ")
    {
        have_password = false;
        // points to static pw buffer after reading the input.
        char * pw = getpass(prompt.c_str());
        if(!pw || (std::strlen(pw) < pw_minlength))
            return;
        password = pw;
        for(char * p = pw; *p;)
            *p++ = '\0';
        have_password = true;
    }

    ~password_from_stdin()
    {
        std::fill(std::begin(password), std::end(password), '\0');
    }

    operator std::string() const { return password; }
    operator bool() const { return have_password; }

    bool have_password;
    std::string password;
};

}
}
}

#endif

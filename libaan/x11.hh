#ifndef _LIBAAN_X11_HH_
#define _LIBAAN_X11_HH_

#include <chrono>
#include <string>

namespace libaan {

bool add_to_clipboard(const std::string &string,
                      std::chrono::milliseconds try_for);
}

#endif

#pragma once

#include <cstddef>

namespace libaan {

// Read from fd in the buffer buff with maximum length len.
int readall(int fd, void *buff, size_t len);

}

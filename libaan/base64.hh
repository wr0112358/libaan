#include <cstdint>

namespace libaan {
size_t base64decode(const void* in, const size_t in_len, char* out, const size_t out_len);
size_t base64encode(const void* in, const size_t in_len, char* out, const size_t out_len);
}

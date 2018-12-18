#include <cstdint>

using uint = unsigned int;

// native unsigned integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
using size_t = decltype(sizeof(void *));

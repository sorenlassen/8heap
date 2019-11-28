#pragma once

#include <stdint.h>
#include <emmintrin.h>

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef uint16_t u16x8 __attribute__ ((vector_size (sizeof(uint16_t) * 8)));
typedef union {
  u16x8 values;
  __m128i mm;
} v128;

static inline v128 mm2v128(__m128i mm) { return { .mm = mm }; }

#ifdef __cplusplus
inline bool operator==(v128 a, v128 b) {
  for (int i = 0; i < 8; ++i) {
    if (a.values[i] != b.values[i]) return false;
  }
  return true;
}
#endif

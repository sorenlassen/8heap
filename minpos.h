#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint16_t
#include <emmintrin.h> // __m128i
#include <smmintrin.h> // _mm_minpos_epu16

typedef int minpos_type;

static inline minpos_type minpos(__m128i mm) {
  return _mm_cvtsi128_si32(_mm_minpos_epu16(mm));
}

static inline uint16_t minpos_min(minpos_type x) { return x & 0xffff; }

static inline size_t minpos_pos(minpos_type x) { return x >> 16; }

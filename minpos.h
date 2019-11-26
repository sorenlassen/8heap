#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint16_t
#include <emmintrin.h> // __m128i
#include <smmintrin.h> // _mm_minpos_epu16

typedef int minpos_type;

static inline uint16_t minpos_min(minpos_type x) { return x & 0xffff; }

static inline size_t minpos_pos(minpos_type x) { return x >> 16; }

static inline minpos_type minpos_shift(minpos_type x, int i) { return x + (i << 16); }

static inline minpos_type minpos(__m128i mm) {
  return _mm_cvtsi128_si32(_mm_minpos_epu16(mm));
}

static inline minpos_type minpos8(uint16_t const* a) {
  return minpos(*(__m128i const*)a);
}

static inline minpos_type minpos16(uint16_t const* a) {
  minpos_type mp0 = minpos8(a);
  minpos_type mp1 = minpos8(a + 8);
  return minpos_min(mp0) <= minpos_min(mp1) ? mp0 : minpos_shift(mp1, 8);
}

static inline minpos_type minpos32(uint16_t const* a) {
  minpos_type mp0 = minpos16(a);
  minpos_type mp1 = minpos16(a + 16);
  return minpos_min(mp0) <= minpos_min(mp1) ? mp0 : minpos_shift(mp1, 16);
}

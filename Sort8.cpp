/*
   Branch free sorting of 8 uint16 numbers with _mm_minpos_epu16.
*/

extern "C" {
#include "minpos.h"
}
#include <cassert>
#include <cstddef> // size_t
#include <cstdint> // uint16_t
#include <cstdlib> // atoi
#include <limits>
#include <emmintrin.h> // __m128i

using namespace std;

namespace {

constexpr uint16_t kMax = std::numeric_limits<uint16_t>::max();
constexpr size_t kArity = 8;

static_assert(sizeof(__m128i) == kArity * sizeof(uint16_t));

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef uint16_t u16x8 __attribute__ ((vector_size (kArity * sizeof(uint16_t))));
typedef union {
  u16x8 values;
  __m128i mm;
} v128;

u16x8 mm2u16x8(__m128i mm) { return reinterpret_cast<u16x8>(mm); }

constexpr v128 kMasks[8] = {
  { { kMax, 0, 0, 0, 0, 0, 0, 0 } },
  { { 0, kMax, 0, 0, 0, 0, 0, 0 } },
  { { 0, 0, kMax, 0, 0, 0, 0, 0 } },
  { { 0, 0, 0, kMax, 0, 0, 0, 0 } },
  { { 0, 0, 0, 0, kMax, 0, 0, 0 } },
  { { 0, 0, 0, 0, 0, kMax, 0, 0 } },
  { { 0, 0, 0, 0, 0, 0, kMax, 0 } },
  { { 0, 0, 0, 0, 0, 0, 0, kMax } },
};

} // namespace

__m128i sort8(__m128i mm) {
  v128 r;
  uint16_t last = 0;
  for (int i = 0; i < kArity; ++i) {
    minpos_type x = minpos(mm);
    uint16_t m = minpos_min(x);
    assert(m >= last);
    last = m;
    r.values[i] = m;
    mm |= kMasks[minpos_pos(x)].mm;
  }
  for (int i = 0; i < kArity; ++i) {
    assert(mm2u16x8(mm)[i] == kMax);
  }
  return r.mm;
}

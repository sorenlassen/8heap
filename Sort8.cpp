/*
   Branch free sorting of 8 uint16 numbers with _mm_minpos_epu16.
*/

#include "minpos.h"
#include "v128.h"
#include <cstddef> // size_t
#include <cstdint> // uint16_t
#include <limits>
#include <emmintrin.h> // __m128i

using namespace std;

namespace {

constexpr uint16_t kMax = std::numeric_limits<uint16_t>::max();
constexpr size_t kArity = 8;

static_assert(sizeof(__m128i) == kArity * sizeof(uint16_t));

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
  for (int i = 0; i < kArity; ++i) {
    minpos_type x = minpos(mm);
    uint16_t m = minpos_min(x);
    r.values[i] = m;
    mm |= kMasks[minpos_pos(x)].mm;
  }
  return r.mm;
}

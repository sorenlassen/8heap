/*
   Branch free sorting of 8 uint16 numbers with _mm_minpos_epu16.

   g++ -g -std=c++17 -msse4 sort8.cpp
*/

#include <cassert>
#include <cstddef> // size_t
#include <cstdint> // uint16_t
#include <limits>
#include <iostream>
#include <emmintrin.h> // __m128i
#include <immintrin.h>
#include <smmintrin.h>

using namespace std;

namespace {

typedef uint16_t value_type;
constexpr value_type kValueMax = std::numeric_limits<value_type>::max();

constexpr size_t arity = 8;
constexpr size_t align = 16; // arity * sizeof(value_type)

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef value_type value_vector __attribute__ ((vector_size (align)));
typedef union {
  value_vector values;
  __m128i mm;
} v128;

typedef int minpos_type;
static minpos_type minpos(v128 v) {
  return _mm_cvtsi128_si32(_mm_minpos_epu16(v.mm));
}
static value_type minpos_min(minpos_type x) { return (uint16_t)x; }
static size_t minpos_pos(minpos_type x) { return x >> 16; }

static_assert(alignof(v128) == align,
              "v128 alignment should be 16");
static_assert(sizeof(v128) == arity * sizeof(value_type),
              "8 values should fill up v128");

constexpr value_vector kMasks[8] = {
  { kValueMax, 0, 0, 0, 0, 0, 0, 0 },
  { 0, kValueMax, 0, 0, 0, 0, 0, 0 },
  { 0, 0, kValueMax, 0, 0, 0, 0, 0 },
  { 0, 0, 0, kValueMax, 0, 0, 0, 0 },
  { 0, 0, 0, 0, kValueMax, 0, 0, 0 },
  { 0, 0, 0, 0, 0, kValueMax, 0, 0 },
  { 0, 0, 0, 0, 0, 0, kValueMax, 0 },
  { 0, 0, 0, 0, 0, 0, 0, kValueMax },
};

value_vector sort8(v128& v) {
  value_vector r;
  minpos_type x[arity];
  value_type last = 0;
  for (int i = 0; i < arity; ++i) {
    auto x = minpos(v);
    value_type m = minpos_min(x);
    assert(m >= last);
    last = m;
    r[i] = m;
    v.values |= kMasks[minpos_pos(x)];
  }
  for (int i = 0; i < arity; ++i) {
    assert(v.values[i] == kValueMax);
  }
  return r;
}

} // namespace

int main(int argc, char** argv) {
  v128 v = { { 7, 9, 13, 1, 2, 3, 4, 5 } };
  value_vector r = sort8(v);
  for (int i = 0; i < arity; ++i) {
    cout << r[i] << " ";
  }
  cout << endl;
  return 0;
}

/*
   # first install gtest as described in h8Test.cpp
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main Sort8.o Sort8Test.cpp
*/

#include "Sort8.hpp"
#include <emmintrin.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <gtest/gtest.h>

namespace {

using namespace std;

constexpr uint16_t kMax = numeric_limits<uint16_t>::max();
constexpr size_t kArity = 8;

static_assert(sizeof(__m128i) == kArity * sizeof(uint16_t));

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef uint16_t u16x8 __attribute__ ((vector_size (kArity * sizeof(uint16_t))));
typedef union {
  u16x8 values;
  __m128i mm;
} v128;

bool operator==(v128 a, v128 b) {
  for (int i = 0; i < kArity; ++i) {
    if (a.values[i] != b.values[i]) return false;
  }
  return true;
}

v128 v128sort(v128 v) {
  v128 r;
  r.mm = sort8(v.mm);
  return r;
}

TEST(sort8, sort8) {
  v128 unsorted = { { kMax - 7, 0, kMax - 13, 3, 2, 3, 2, 3 } };
  v128 sorted = { { 0, 2, 2, 3, 3, 3, kMax - 13, kMax - 7 } };
  EXPECT_EQ(sorted, v128sort(unsorted));
  EXPECT_EQ(sorted, v128sort(sorted));
}

} // namespace

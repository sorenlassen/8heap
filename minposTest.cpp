/*
   # first install gtest as described in h8Test.cpp
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main minposTest.cpp
*/

#include "minpos.h"
#include "v128.h"
#include <stdalign.h> // no <cstdalign> on mac
#include <cstdint>
#include <gtest/gtest.h>

namespace {

// SSE register size and memory alignment
#define kAlign 16

TEST(minpos, minpos) {
  constexpr uint16_t M = 65535;
  v128 v0 = { { M - 7, M - 9, M - 13, M - 3, M - 2, M - 3, M - 2, M - 3 } };
  minpos_type mp = minpos(v0.mm);
  EXPECT_EQ(M - 13, minpos_min(mp));
  EXPECT_EQ(2, minpos_pos(mp));
}

TEST(minpos, minpos8) {
  constexpr uint16_t M = 65535;
  alignas(kAlign) uint16_t vs[8] = {
    M - 7, M - 9, M - 13, M - 3, M - 2, M - 3, M - 2, M - 3
  };
  minpos_type mp = minpos8(vs);
  EXPECT_EQ(M - 13, minpos_min(mp));
  EXPECT_EQ(2, minpos_pos(mp));
}

TEST(minpos, minpos16) {
  constexpr uint16_t M = 65535;
  alignas(kAlign) uint16_t vs[16] = {
    M - 7, M - 9, M - 13, M - 3, M - 2, M - 3, M - 2, M - 3,
    M - 107, M - 109, M - 113, M - 13, 12, 13, 12, 13
  };
  minpos_type mp = minpos16(vs);
  EXPECT_EQ(12, minpos_min(mp));
  EXPECT_EQ(12, minpos_pos(mp));
}

TEST(minpos, minpos32) {
  constexpr uint16_t M = 65535;
  alignas(kAlign) uint16_t vs[32] = {
    M - 7, M - 9, M - 13, M - 3, M - 2, M - 3, M - 2, M - 3,
    M - 107, M - 109, M - 113, M - 13, 12, 13, 12, 13,
    7, 9, 13, 3, 2, 3, 2, 3,
    7, 9, 13, 3, M - 2, M - 3, M - 2, M - 3
  };
  minpos_type mp = minpos32(vs);
  EXPECT_EQ(2, minpos_min(mp));
  EXPECT_EQ(20, minpos_pos(mp));
}

} // namespace

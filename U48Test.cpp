/*
   # first install gtest as described in h8Test.cpp
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main U48Test.cpp
   ./a.out
*/

#include "U48.hpp"
#include <cstdint>
#include <limits>
#include <gtest/gtest.h>

namespace {

constexpr U48 kU48Max = std::numeric_limits<U48>::max();
constexpr uint64_t k48Mask = (uint64_t(1) << 48) - 1;

TEST(U48, conversions) {
  EXPECT_EQ(42, static_cast<U48>(42));
  EXPECT_EQ(42, static_cast<U48>(42 + (uint64_t(42) << 48)));
}

TEST(U48, max) {
  EXPECT_EQ(k48Mask, kU48Max);
  EXPECT_EQ(0, static_cast<U48>(k48Mask + 1));
  EXPECT_EQ(kU48Max, static_cast<U48>(std::numeric_limits<uint64_t>::max()));
}

} // namespace

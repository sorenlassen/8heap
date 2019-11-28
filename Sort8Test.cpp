/*
   # first install gtest as described in h8Test.cpp
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main Sort8.o Sort8Test.cpp
*/

#include "Sort8.hpp"
#include "v128.h"
#include <cstdint>
#include <limits>
#include <gtest/gtest.h>

namespace {

using namespace std;

constexpr uint16_t kMax = numeric_limits<uint16_t>::max();

TEST(sort8, sort8) {
  v128 unsorted = { { kMax - 7, 0, kMax - 13, 3, 2, 3, 2, 3 } };
  v128 sorted = { { 0, 2, 2, 3, 3, 3, kMax - 13, kMax - 7 } };
  EXPECT_EQ(sorted, mm2v128(sort8(unsorted.mm)));
  EXPECT_EQ(sorted, mm2v128(sort8(sorted.mm)));
}

} // namespace

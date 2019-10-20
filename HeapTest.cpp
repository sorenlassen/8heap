/*
   # The following instructions to install gtest on Mac are adapted from:
   #   https://stackoverflow.com/a/46611467
   #   https://github.com/google/googletest/blob/master/googletest/README.md
   brew install cmake
   pushd $HOME # or whereever you want to clone googletest
   git clone https://github.com/google/googletest.git
   cd googletest
   mkdir build
   cd build/
   cmake ..
   make
   make install
   popd

   gcc -g -std=c11 -msse4 -c h8.c &&
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main h8.o HeapTest.cpp
*/

#include "H8.hpp"
#include "Heap8.hpp"
#include "StdMinHeap.hpp"
#include <vector>
#include <gtest/gtest.h>

namespace {

using testing::Types;

template <class T>
class HeapTest : public testing::Test {
 protected:
  T heap_;
};

typedef Types<H8, h8::Heap8, StdMinHeap> Implementations;

TYPED_TEST_SUITE(HeapTest, Implementations);

TYPED_TEST(HeapTest, Push3) {
  EXPECT_EQ(0, this->heap_.size());
  this->heap_.push(2);
  EXPECT_EQ(1, this->heap_.size());
  EXPECT_EQ(2, this->heap_.top());
  this->heap_.push(1);
  EXPECT_EQ(2, this->heap_.size());
  EXPECT_EQ(1, this->heap_.top());
  this->heap_.push(3);
  EXPECT_EQ(3, this->heap_.size());
  EXPECT_EQ(1, this->heap_.top());
}

TYPED_TEST(HeapTest, Heapify3) {
  std::vector<elem_type> values = {1, 2, 3};
  this->heap_.append(values.begin(), values.end());
  this->heap_.heapify();
  EXPECT_EQ(1, this->heap_.pop());
  EXPECT_EQ(2, this->heap_.pop());
  EXPECT_EQ(3, this->heap_.pop());
}

} // namespace

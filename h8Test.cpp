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
   g++ -g -std=c++14 -msse4 -lgtest -lgtest_main h8.o h8Test.cpp
*/

#include <cstddef>
#include <limits>
extern "C" {
#include "h8.h"
}
#include <gtest/gtest.h>

namespace {

constexpr size_t arity = 8;

size_t parent(size_t q) { return (q / arity) - 1; }

TEST(h8, heap_init) {
  heap h;
  heap_init(&h);
  EXPECT_EQ(nullptr, h.array);
  EXPECT_EQ(0, h.capacity);
  EXPECT_EQ(0, h.size);
}

TEST(h8, heap_clear) {
  heap h;
  heap_init(&h);
  value_type* ptr = heap_extend(&h, 3);
  heap_clear(&h);
  EXPECT_EQ(nullptr, h.array);
  EXPECT_EQ(0, h.capacity);
  EXPECT_EQ(0, h.size);
}

TEST(h8, heap_extend) {
  heap h;
  heap_init(&h);
  value_type* ptr = heap_extend(&h, 3);
  EXPECT_NE(nullptr, ptr);
  EXPECT_EQ(ptr, h.array);
  EXPECT_GE(h.capacity, 3);
  EXPECT_EQ(3, h.size);
  heap_clear(&h);
}

TEST(h8, heap_extend_too_much) {
  heap h;
  heap_init(&h);
  EXPECT_TRUE(heap_push(&h, 42));
  EXPECT_EQ(1, h.size);
  EXPECT_EQ(42, heap_top(&h));
  EXPECT_EQ(nullptr, heap_extend(&h, std::numeric_limits<size_t>::max()));
  EXPECT_EQ(1, h.size);
  EXPECT_EQ(42, heap_top(&h));
  heap_clear(&h);
}

TEST(h8, heap_heapify_3) {
  heap h;
  heap_init(&h);
  value_type* ptr = heap_extend(&h, 3);
  ptr[0] = 2;
  ptr[1] = 1;
  ptr[2] = 3;
  heap_heapify(&h);
  EXPECT_EQ(1, heap_pop(&h));
  EXPECT_EQ(2, heap_pop(&h));
  EXPECT_EQ(3, heap_pop(&h));
  heap_clear(&h);
}

TEST(h8, heap_heapify_100) {
  heap h;
  heap_init(&h);
  size_t const n = 100;
  value_type* ptr = heap_extend(&h, n);
  EXPECT_NE(nullptr, ptr);
  EXPECT_EQ(n, h.size);
  for (size_t i = 0; i < n; ++i) ptr[i] = n - 1 - i;
  heap_heapify(&h);
  EXPECT_TRUE(heap_is_heap(&h));
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(i, heap_pop(&h));
  }
  heap_clear(&h);
}

TEST(h8, heap_push_3) {
  heap h;
  heap_init(&h);
  EXPECT_TRUE(heap_push(&h, 2));
  EXPECT_NE(nullptr, h.array);
  EXPECT_GE(h.capacity, 1);
  EXPECT_EQ(1, h.size);
  EXPECT_EQ(2, heap_top(&h));
  EXPECT_TRUE(heap_push(&h, 1));
  EXPECT_EQ(1, heap_top(&h));
  EXPECT_TRUE(heap_push(&h, 3));
  EXPECT_EQ(1, heap_top(&h));
  heap_clear(&h);
}

TEST(h8, heap_push_100) {
  heap h;
  heap_init(&h);
  size_t const n = 100;
  for (size_t i = 0; i < n; ++i) {
    EXPECT_TRUE(heap_push(&h, n - 1 - i));
  }
  heap_heapify(&h);
  EXPECT_TRUE(heap_is_heap(&h));
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(i, heap_pop(&h));
  }
  heap_clear(&h);
}

} // namespace

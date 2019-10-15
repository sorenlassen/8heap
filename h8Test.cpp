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

   gcc -g -std=c11 -msse4 -c h8.c
   g++ -g -std=c++14 -msse4 -lgtest -lgtest_main h8.o h8Test.cpp
*/

#include <limits>
extern "C" {
#include "h8.h"
}
#include <gtest/gtest.h>

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
  elem_type* ptr = heap_extend(&h, 3);
  heap_clear(&h);
  EXPECT_EQ(nullptr, h.array);
  EXPECT_EQ(0, h.capacity);
  EXPECT_EQ(0, h.size);
}

TEST(h8, heap_extend) {
  heap h;
  heap_init(&h);
  elem_type* ptr = heap_extend(&h, 3);
  EXPECT_NE(nullptr, ptr);
  EXPECT_EQ(ptr, h.array);
  EXPECT_GE(h.capacity, 3);
  EXPECT_EQ(3, h.size);
  heap_clear(&h);
}

TEST(h8, heap_heapify) {
  heap h;
  heap_init(&h);
  elem_type* ptr = heap_extend(&h, 3);
  ptr[0] = 1;
  ptr[1] = 3;
  ptr[2] = 2;
  heap_heapify(&h);
  EXPECT_EQ(1, heap_pop(&h));
  EXPECT_EQ(2, heap_pop(&h));
  EXPECT_EQ(3, heap_pop(&h));
  heap_clear(&h);
}

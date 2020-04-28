/*
   # first install gtest as described in h8Test.cpp
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main HeapAuxTest.cpp
   ./a.out
*/

#include "H8.hpp"
#include "Heap8.hpp"
#include "Heap8Aux.hpp"
#include "StdMinHeap.hpp"
#include <vector>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <gtest/gtest.h>

namespace {

using boost::iterators::counting_iterator;
using boost::iterators::transform_iterator;
using testing::Types;

typedef Heap8Aux<int> Aux;

template <class T>
class HeapAuxTest : public testing::Test {
 protected:
  T heap_;
};

typedef Types<Aux> Implementations;

TYPED_TEST_SUITE(HeapAuxTest, Implementations);

TYPED_TEST(HeapAuxTest, Clear) {
  EXPECT_EQ(0, this->heap_.size());
  this->heap_.push_entry(1, 100);
  EXPECT_EQ(1, this->heap_.size());
  this->heap_.clear();
  EXPECT_EQ(0, this->heap_.size());
}

TYPED_TEST(HeapAuxTest, Push3) {
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(2, 200);
  EXPECT_EQ(1, this->heap_.size());
  auto ht = this->heap_.top_entry();
  EXPECT_EQ(2, ht.first);
  EXPECT_EQ(200, ht.second);
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(1, 100);
  ht = this->heap_.top_entry();
  EXPECT_EQ(1, ht.first);
  EXPECT_EQ(100, ht.second);
  EXPECT_EQ(2, this->heap_.size());
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(3, 300);
  ht = this->heap_.top_entry();
  EXPECT_EQ(1, ht.first);
  EXPECT_EQ(100, ht.second);
  EXPECT_EQ(3, this->heap_.size());
  EXPECT_TRUE(this->heap_.is_heap());
}

TYPED_TEST(HeapAuxTest, Heapify3) {
  typedef typename TypeParam::entry_type entry_type;
  entry_type p1(1, 41), p2(2, 42), p3(3, 43);
  std::vector<entry_type> entries{p2, p1, p3};
  this->heap_.append_entries(entries.begin(), entries.end());
  EXPECT_EQ(entries.size(), this->heap_.size());
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(p1, this->heap_.pop_entry());
  EXPECT_EQ(p2, this->heap_.pop_entry());
  EXPECT_EQ(p3, this->heap_.pop_entry());
}

TYPED_TEST(HeapAuxTest, Sort3) {
  typedef typename TypeParam::entry_type entry_type;
  entry_type p1(1, 41), p2(2, 42), p3(3, 43);
  std::vector<entry_type> entries{p2, p1, p3};
  this->heap_.append_entries(entries.begin(), entries.end());
  this->heap_.heapify();
  this->heap_.sort();
  EXPECT_EQ(0, this->heap_.size());
  EXPECT_EQ(p3, this->heap_.entry(0));
  EXPECT_EQ(p2, this->heap_.entry(1));
  EXPECT_EQ(p1, this->heap_.entry(2));
  EXPECT_TRUE(this->heap_.is_sorted(entries.size()));
}

TYPED_TEST(HeapAuxTest, Heapify100) {
  typedef typename TypeParam::value_type value_type;
  typedef typename TypeParam::entry_type entry_type;
  value_type const count = 100;
  counting_iterator<value_type> zero(0);
  auto revert = [=](value_type i) {
    value_type j = count - 1 - i;
    return entry_type(j, 40 + j);
  };
  auto begin = transform_iterator(zero, revert);
  this->heap_.append_entries(begin, begin + count);
  EXPECT_EQ(count, this->heap_.size());
  // 8 is the arity of Heap8Aux (dirty implementation detail, oh well)
  EXPECT_LE(count - 8, this->heap_.top_entry().first);
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(entry_type(0, 40), this->heap_.top_entry());
  for (value_type i = 0; i < count; ++i) {
    EXPECT_EQ(entry_type(i, i + 40), this->heap_.pop_entry());
  }
}

} // namespace

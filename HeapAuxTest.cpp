/*
   # first install gtest as described in h8Test.cpp
   gcc -g -std=c11 -msse4 -c h8.c &&
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main h8.o HeapTest.cpp
*/

#include "H8.hpp"
#include "Heap8.hpp"
#include "Heap8Aux.hpp"
#include "StdMinHeap.hpp"
#include <utility>
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
class HeapAux : public testing::Test {
 protected:
  T heap_;
};

typedef Types<Aux> Implementations;

TYPED_TEST_SUITE(HeapAux, Implementations);

TYPED_TEST(HeapAux, Clear) {
  EXPECT_EQ(0, this->heap_.size());
  this->heap_.push_entry(1, 100);
  EXPECT_EQ(1, this->heap_.size());
  this->heap_.clear();
  EXPECT_EQ(0, this->heap_.size());
}

TYPED_TEST(HeapAux, Push3) {
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(2, 200);
  EXPECT_EQ(1, this->heap_.size());
  std::pair<uint32_t, int> ht = this->heap_.top_entry();
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


/*
TEST(Heap8Aux, Clear) {
  Heap8Aux<int> heap_ = Heap8Aux<int>(); 
  EXPECT_EQ(0, heap_.size());
  heap_.push_entry(1, 32);
  EXPECT_EQ(1, heap_.size());
  heap_.clear();
  EXPECT_EQ(0, heap_.size());
}

TYPED_TEST(Heap8Aux, Push3) {
  Heap8Aux<int> heap_ = Heap8Aux<int>(); 
  EXPECT_TRUE(heap_.is_heap());
  heap_.push(2, 200);
  EXPECT_EQ(1, heap_.size());
  
  EXPECT_EQ(2, this->heap_.top_entry());
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push(1);
  EXPECT_EQ(2, this->heap_.size());
  EXPECT_EQ(1, this->heap_.top_entry());
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push(3);
  EXPECT_EQ(3, this->heap_.size());
  EXPECT_EQ(1, this->heap_.top_entry());
  EXPECT_TRUE(this->heap_.is_heap());
}

*/
/*
TYPED_TEST(HeapTest, Heapify3) {
  typedef typename TypeParam::value_type value_type;
  std::vector<value_type> values{2, 1, 3};
  this->heap_.append(values.begin(), values.end());
  EXPECT_EQ(values.size(), this->heap_.size());
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(1, this->heap_.pop());
  EXPECT_EQ(2, this->heap_.pop());
  EXPECT_EQ(3, this->heap_.pop());
}

TYPED_TEST(HeapTest, Sort3) {
  typedef typename TypeParam::value_type value_type;
  std::vector<value_type> values{2, 1, 3};
  this->heap_.append(values.begin(), values.end());
  this->heap_.heapify();
  this->heap_.sort();
  EXPECT_EQ(0, this->heap_.size());
  EXPECT_EQ(3, this->heap_[0]);
  EXPECT_EQ(2, this->heap_[1]);
  EXPECT_EQ(1, this->heap_[2]);
  EXPECT_TRUE(this->heap_.is_sorted(values.size()));
}

TYPED_TEST(HeapTest, Heapify100) {
  typedef typename TypeParam::value_type value_type;
  value_type const count = 100;
  counting_iterator<value_type> zero(0);
  auto revert = [=](value_type i) { return count - 1 - i; };
  auto begin = transform_iterator(zero, revert);
  this->heap_.append(begin, begin + count);
  EXPECT_EQ(count, this->heap_.size());
  // 8 is the arity of H8 and Heap8 (dirty implementation detail, oh well)
  EXPECT_LE(count - 8, this->heap_.top_entry());
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(0, this->heap_.top_entry());
  for (value_type i = 0; i < count; ++i) {
    EXPECT_EQ(i, this->heap_.pop());
  }
}
*/

} // namespace

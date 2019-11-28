#pragma once

#include "minpos.h"
#include "v128.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <new>
#include <vector>

namespace h8 {

class Heap8 {
 public:
  typedef std::uint16_t value_type;
  typedef std::size_t size_type;

 private:
  static constexpr value_type kMax = std::numeric_limits<value_type>::max();
  static constexpr size_type kArity = 8;
  static constexpr size_type kSizeMax =
    std::numeric_limits<size_type>::max() - (kArity - 1);

  static size_type parent(size_type q) { return (q / kArity) - 1; }
  static size_type children(size_type p) { return (p + 1) * kArity; }

  static_assert(sizeof(v128) == kArity * sizeof(value_type));

 public:
  Heap8() : size_(0) { }
  ~Heap8() = default;
  Heap8(const Heap8&) = delete;
  Heap8& operator=(const Heap8&) = delete;

  size_type size() const { return size_; }

  value_type& operator[](size_type index) {
    return data()[index];
  }

  value_type* extend(size_type n) {
    if (n > kSizeMax - size_) throw_bad_alloc();
    size_type new_size = size_ + n;
    if (new_size > kArity * vectors_.size()) {
      static_assert(std::numeric_limits<vectors_type::size_type>::max() >=
                    std::numeric_limits<size_type>::max() / kArity);
      // Smallest new_vectors_size s.t. size <= kArity * new_vectors_size.
      size_type new_vectors_size = (new_size + (kArity - 1)) / kArity;
      vectors_.resize(new_vectors_size, kV128Max);
    }
    size_ = new_size;
    value_type* array = data();
    return array + (size_ - n);
  }

  template<class InputIterator>
  void append(InputIterator begin, InputIterator end) {
    value_type* array = data();
    while (begin != end) {
      if (size_ == kArity * vectors_.size()) {
        vectors_.push_back(kV128Max);
        array = data();
      }
      array[size_++] = *begin++;
    }
  }

  void pull_up(value_type b, size_type q) {
    assert(q < size_);
    value_type* array = data();
    while (q >= kArity) {
      size_t p = parent(q);
      value_type a = array[p];
      if (a <= b) break;
      array[q] = a;
      q = p;
    }
    array[q] = b;
  }

  void push_down(value_type a, size_type p) {
    assert(p < size_);
    value_type* array = data();
    while (true) {
      size_t q = children(p);
      if (q >= size_) break;
      minpos_type x = minpos(vectors_[q / kArity].mm);
      value_type b = minpos_min(x);
      if (a <= b) break;
      array[p] = b;
      p = q + minpos_pos(x);
    }
    array[p] = a;
  }

  void heapify() {
    if (size_ <= kArity) return;
    value_type* array = data();
    size_t q = (size_ - 1) & ~(kArity - 1); // align_down(size_ - 1, kArity);

    // The first while loop is an optimization for the bottom level of the heap,
    // inlining the call to heap_push_down which is trivial at the bottom level.
    // Here "bottom level" means the 8-vectors without children.
    size_t r = parent(q);
    while (q > r) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      value_type b = minpos_min(x);
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) {
        array[p] = b;
        // The next line inlines heap_push_down(h, a, q + minpos_pos(x))
        // with the knowledge that children(q) >= h->size.
        array[q + minpos_pos(x)] = a;
      }
      q -= kArity;
    }

    while (q > 0) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      value_type b = minpos_min(x);
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) {
        array[p] = b;
        push_down(a, q + minpos_pos(x));
      }
      q -= kArity;
    }
  }

  bool is_heap() const {
    if (size_ <= kArity) return true;
    value_type const* array = data();
    size_t q = (size_ - 1) & ~(kArity - 1); // align_down(size_ - 1, kArity);
    while (q > 0) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      value_type b = minpos_min(x);
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) return false;
      q -= kArity;
    }
    return true;
  }

  void push(value_type b) {
    if (size_ == kArity * vectors_.size()) vectors_.push_back(kV128Max);
    size_++;
    pull_up(b, size_ - 1);
  }

  value_type const top() {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0].mm);
    return minpos_min(x);
  }

  value_type pop() {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0].mm);
    value_type b = minpos_min(x);
    value_type* array = data();
    value_type a = array[size_ - 1];
    array[size_ - 1] = kMax;
    size_--;
    size_type p = minpos_pos(x);
    if (p != size_) {
      push_down(a, p);
    }
    return b;
  }

  void sort() {
    v128 v = kV128Max;
    size_type x = size_;
    size_type i = x % kArity;
    x -= i;
    while (i > 0) {
      --i;
      v.values[i] = pop();
    }
    vectors_[x / kArity] = v;
    while (x > 0) {
      x -= kArity;
      for (size_type j = kArity; j > 0; --j) {
        v.values[j - 1] = pop();
      }
      vectors_[x / kArity] = v;
    }
  }

  bool is_sorted(size_type sz) const {
    return std::is_sorted(data(), data() + sz, std::greater<value_type>());
  }

  void clear() {
    vectors_.clear();
    vectors_.shrink_to_fit(); // to match heap_clear(heap*)
    size_ = 0;
  }

 private:
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  value_type* data() { return reinterpret_cast<value_type*>(vectors_.data()); }
  value_type const* data() const { return reinterpret_cast<value_type const*>(vectors_.data()); }
  typedef std::vector<v128> vectors_type;
  vectors_type vectors_;
  size_type size_;
};

} // namespace h8

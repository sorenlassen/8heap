#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
#include <emmintrin.h>
#include <smmintrin.h>

namespace h8 {

class Heap8 {
 public:
  typedef std::uint16_t elem_type;
  typedef std::size_t size_type;

 private:
  static constexpr elem_type elem_max = std::numeric_limits<elem_type>::max();

  static constexpr size_type arity = 8;
  static size_type parent(size_type q) { return (q / arity) - 1; }
  static size_type children(size_type p) { return (p + 1) * arity; }

  static constexpr size_type align = 16;
  // https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
  typedef elem_type elem_vector __attribute__ ((vector_size (align)));
  union v128 {
    __m128i mm;
    elem_vector elems;
  };
  static_assert(alignof(v128) == align);
  static_assert(sizeof(v128) == arity * sizeof(elem_type));
  // The following is gcc specific, see: https://stackoverflow.com/a/35268748
  // whereas _mm_set1_epi16(elem_max) would be more cross platform.
  static constexpr v128 v128_max = { { ~0LL, ~0LL } };

  typedef int minpos_type;
  static minpos_type minpos(v128 v) {
    return _mm_cvtsi128_si32(_mm_minpos_epu16(v.mm));
  }
  static elem_type minpos_min(minpos_type x) { return static_cast<std::uint16_t>(x); }
  static size_type minpos_pos(minpos_type x) { return x >> 16; }

 public:
  Heap8() : size_(0) { }
  ~Heap8() = default;

  size_type size() const { return size_; }

  elem_type* extend(size_type n) {
    size_ += n;
    if (size_ > arity * vectors_.size()) {
      // Smallest new_vectors_size s.t. size <= arity * new_vectors_size.
      size_type new_vectors_size = (size_ + arity - 1) / arity;
      vectors_.resize(new_vectors_size, v128_max);
    }
    elem_type* array = data();
    return array + (size_ - n);
  }

  template<class InputIterator>
  void append(InputIterator begin, InputIterator end) {
    elem_type* array = data();
    while (begin != end) {
      if (size_ == arity * vectors_.size()) {
        vectors_.push_back(v128_max);
        array = data();
      }
      array[size_++] = *begin++;
    }
  }

  void pull_up(elem_type b, size_type q) {
    assert(q < size_);
    elem_type* array = data();
    while (q >= arity) {
      size_t p = parent(q);
      elem_type a = array[p];
      if (a <= b) break;
      array[q] = a;
      q = p;
    }
    array[q] = b;
  }

  void push_down(elem_type a, size_type p) {
    assert(p < size_);
    elem_type* array = data();
    while (true) {
      size_t q = children(p);
      if (q >= size_) break;
      minpos_type x = minpos(vectors_[q / arity]);
      elem_type b = minpos_min(x);
      if (a <= b) break;
      array[p] = b;
      p = q + minpos_pos(x);
    }
    array[p] = a;
  }

  void heapify() {
    if (size_ <= arity) return;
    elem_type* array = data();
    size_t q = (size_ - 1) & ~(arity - 1); // align_down(size_ - 1, arity);

    // The first while loop is an optimization for the bottom level of the heap,
    // inlining the call to heap_push_down which is trivial at the bottom level.
    // Here "bottom level" means the 8-vectors without children.
    size_t r = parent(q);
    while (q > r) {
      minpos_type x = minpos(vectors_[q / arity]);
      elem_type b = minpos_min(x);
      size_t p = parent(q);
      elem_type a = array[p];
      if (b < a) {
        array[p] = b;
        // The next line inlines heap_push_down(h, a, q + minpos_pos(x))
        // with the knowledge that children(q) >= h->size.
        array[q + minpos_pos(x)] = a;
      }
      q -= arity;
    }

    while (q > 0) {
      minpos_type x = minpos(vectors_[q / arity]);
      elem_type b = minpos_min(x);
      size_t p = parent(q);
      elem_type a = array[p];
      if (b < a) {
        array[p] = b;
        push_down(a, q + minpos_pos(x));
      }
      q -= arity;
    }
  }

  void push(elem_type b) {
    if (size_ == arity * vectors_.size()) vectors_.push_back(v128_max);
    size_++;
    pull_up(b, size_ - 1);
  }

  elem_type const top() {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0]);
    return minpos_min(x);
  }

  elem_type pop() {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0]);
    elem_type b = minpos_min(x);
    elem_type* array = data();
    elem_type a = array[size_ - 1];
    array[size_ - 1] = elem_max;
    size_--;
    size_type p = minpos_pos(x);
    if (p != size_) {
      push_down(a, p);
    }
    return b;
  }

  void clear() {
    std::vector<v128> empty;
    vectors_.swap(empty);
    size_ = 0;
  }

 private:
  elem_type* data() { return reinterpret_cast<elem_type*>(vectors_.data()); }
  elem_type const* data() const { return reinterpret_cast<elem_type const*>(vectors_.data()); }

  std::vector<v128> vectors_;
  size_type size_;
};

} // namespace h8

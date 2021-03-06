#pragma once

#include "minpos.h"
#include "v128.h"
#include "align.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <new>
#include <utility>
#include <functional>
#include <vector>

template<class S> class Heap8Aux {
 public:
  typedef std::uint16_t key_type;
  typedef S mapped_type;
  typedef std::pair<key_type, S> entry_type;
  typedef std::size_t size_type;

 private:
  static constexpr key_type kMax = std::numeric_limits<key_type>::max();
  static constexpr size_type kArity = 8;
  static constexpr size_type kSizeMax = align_down(std::numeric_limits<size_type>::max(), kArity);

  static size_type parent(size_type q) { return (q / kArity) - 1; }
  static size_type children(size_type p) { return (p + 1) * kArity; }

  static_assert(sizeof(v128) == kArity * sizeof(key_type));

 public:
  Heap8Aux() : size_(0) { }
  ~Heap8Aux() = default;
  Heap8Aux(const Heap8Aux&) = delete;
  Heap8Aux& operator=(const Heap8Aux&) = delete;

  size_type size() const { return size_; }

  key_type key(size_type index) const { return data()[index]; }

  entry_type entry(size_type index) const {
    return std::make_pair(data()[index], shadow_[index]);
  }

  void set_entry(size_type index, entry_type a) {
    data()[index] = a.first;
    shadow_[index] = a.second;
  }

  void extend(size_type n) {
    if (n > kSizeMax - size_) throw_bad_alloc();
    size_type new_size = size_ + n;
    if (new_size > kArity * vectors_.size()) {
      static_assert(std::numeric_limits<vectors_type::size_type>::max() >=
                    std::numeric_limits<size_type>::max() / kArity);
      // Smallest new_vectors_size s.t. size <= kArity * new_vectors_size.
      size_type new_vectors_size = align_up(new_size, kArity) / kArity;
      vectors_.resize(new_vectors_size, kV128Max);
      shadow_.resize(new_size);
    }
    size_ = new_size;
    key_type* array = data();
  }

  template<class InputIterator>
  void append_entries(InputIterator begin, InputIterator end) {
    key_type* array = data();
    while (begin != end) {
      if (size_ == kArity * vectors_.size()) {
        vectors_.push_back(kV128Max);
        array = data();
      }
      array[size_] = begin->first;
      shadow_.push_back(begin->second);
      ++begin;
      ++size_;
    }
  }

  void pull_up(key_type b, mapped_type t, size_type q) {
    assert(q < size_);
    key_type* array = data();
    while (q >= kArity) {
      size_type p = parent(q);
      key_type a = array[p];
      if (a <= b) break;
      array[q] = a;
      shadow_[q] = shadow_[p];
      q = p;
    }
    array[q] = b;
    shadow_[q] = t;
  }

  void push_down(key_type a, mapped_type s, size_type p) {
    assert(p < size_);
    key_type* array = data();
    while (true) {
      size_type q = children(p);
      if (q >= size_) break;
      minpos_type x = minpos(vectors_[q / kArity].mm);
      key_type b = minpos_min(x);
      if (a <= b) break;
      array[p] = b;
      q += minpos_pos(x);
      shadow_[p] = shadow_[q];
      p = q;
    }
    array[p] = a;
    shadow_[p] = s;
  }

  void heapify() {
    if (size_ <= kArity) return;
    key_type* array = data();
    size_type q = align_down(size_ - 1, kArity);

    // The first while loop is an optimization for the bottom level of the heap,
    // inlining the call to heap_push_down which is trivial at the bottom level.
    // Here "bottom level" means the 8-vectors without children.
    size_type r = parent(q);
    while (q > r) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      key_type b = minpos_min(x);
      size_type p = parent(q);
      key_type a = array[p];
      if (b < a) {
        size_type q_new = q + minpos_pos(x);
        mapped_type s = shadow_[p];
        shadow_[p] = shadow_[q_new];
        array[p] = b;
        // The next line inlines push_down(a, s, q_new)
        // with the knowledge that children(q_new) >= size_.
        array[q_new] = a;
        shadow_[q_new] = s;
      }
      q -= kArity;
    }

    while (q > 0) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      key_type b = minpos_min(x);
      size_type p = parent(q);
      key_type a = array[p];
      if (b < a) {
        size_type q_new = q + minpos_pos(x);
        mapped_type s = shadow_[p];
        shadow_[p] = shadow_[q_new];
        array[p] = b;
        push_down(a, s, q_new);
      }
      q -= kArity;
    }
  }

  bool is_heap() const {
    if (size_ <= kArity) return true;
    key_type const* array = data();
    size_type q = align_down(size_ - 1, kArity);
    while (q > 0) {
      minpos_type x = minpos(vectors_[q / kArity].mm);
      key_type b = minpos_min(x);
      size_type p = parent(q);
      key_type a = array[p];
      if (b < a) return false;
      q -= kArity;
    }
    return true;
  }

  void push_entry(entry_type e) {
    push_entry(e.first, e.second);
  }

  void push_entry(key_type b, mapped_type t) {
    if (size_ == kArity * vectors_.size()) vectors_.push_back(kV128Max);
    size_++;
    shadow_.push_back(t); // to grow shadow_; pull_up overwrites the value
    pull_up(b, t, size_ - 1);
  }

  size_type top_index() const {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0].mm);
    return minpos_pos(x);
  }

  entry_type top_entry() const {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0].mm);
    return std::make_pair(minpos_min(x), shadow_[minpos_pos(x)]);
  }

  entry_type pop_entry() {
    assert(size_ > 0);
    minpos_type x = minpos(vectors_[0].mm);
    size_type q = minpos_pos(x);
    entry_type e(minpos_min(x), shadow_[q]);
    key_type* array = data();
    key_type a = array[size_ - 1];
    array[size_ - 1] = kMax;
    size_--;
    if (q != size_) {
      mapped_type s = shadow_[size_];
      push_down(a, s, q);
    }
    shadow_.pop_back();
    return e;
  }

  void sort() {
    v128 v = kV128Max;
    size_type x = size_;
    size_type i = x % kArity;
    x -= i;
    if (i != 0) {
      do {
        --i;
        entry_type e = pop_entry();
        v.values[i] = e.first;
        shadow_[x + i] = e.second;
      } while (i > 0);
      vectors_[x / kArity] = v;
    }
    while (x > 0) {
      x -= kArity;
      for (size_type j = kArity; j > 0; --j) {
        entry_type e = pop_entry();
        v.values[j - 1] = e.first;
        shadow_[x + j - 1] = e.second;
      }
      vectors_[x / kArity] = v;
    }
  }

  bool is_sorted(size_type sz) const {
    return std::is_sorted(data(), data() + sz, std::greater<key_type>());
  }

  void clear() {
    vectors_.clear();
    shadow_.clear();
    vectors_.shrink_to_fit(); // to match heap_clear(heap*)
    shadow_.shrink_to_fit();
    size_ = 0;
  }

 private:
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  key_type* data() { return reinterpret_cast<key_type*>(vectors_.data()); }
  key_type const* data() const { return reinterpret_cast<key_type const*>(vectors_.data()); }
  typedef std::vector<v128> vectors_type;
  vectors_type vectors_;
  std::vector<S> shadow_;
  size_type size_;
};

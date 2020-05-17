#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <new>
#include <vector>

template<class V = uint16_t, class Compare = std::greater<V>>
class StdMinHeap {
 public:
  typedef V value_type;
 private:
  typedef std::vector<value_type> array_type;
 public:
  typedef typename array_type::size_type size_type;

  StdMinHeap(const Compare& cmp = Compare()) : cmp_(cmp) { }
  ~StdMinHeap() = default;
  StdMinHeap(const StdMinHeap&) = delete;
  StdMinHeap& operator=(const StdMinHeap&) = delete;

  size_type size() const { return array_.size(); }

  value_type& operator[](size_type index) { return array_[index]; }

  value_type const& operator[](size_type index) const { return array_[index]; }

  value_type* extend(size_type n) {
    size_type old_size = array_.size();
    if (n > std::numeric_limits<size_type>::max() - old_size) throw_bad_alloc();
    array_.resize(old_size + n);
    return &array_[old_size];
  }

  template<class InputIterator>
  void append(InputIterator begin, InputIterator end) {
    array_.insert(array_.end(), begin, end);
  }

  void pull_up(value_type b, size_type q) {
    assert(q < size());
    while (q > 0) {
      size_type p = parent(q);
      value_type a = array_[p];
      if (!cmp_(a, b)) break;
      array_[q] = a;
      q = p;
    }
    array_[q] = b;
  }

  void push_down(value_type a, size_type p) {
    size_type sz = size();
    assert(p < sz);
    while (true) {
      size_type q = children(p);
      if (q + 1 > sz) break;
      value_type b = array_[q];
      if (q + 1 < sz) {
        value_type c = array_[q + 1];
        if (cmp_(b, c)) {
          q += 1;
          b = c;
        }
      }
      if (!cmp_(a, b)) break;
      array_[p] = b;
      p = q;
    }
    array_[p] = a;
  }

  void heapify() {
    std::make_heap(array_.begin(), array_.end(), cmp_);
  }

  bool is_heap() const {
    return std::is_heap(array_.begin(), array_.end(), cmp_);
  }

  void push(value_type b) {
    array_.push_back(b);
#ifdef STD_PUSH_HEAP
    std::push_heap(array_.begin(), array_.end(), cmp_);
#else
    pull_up(b, size() - 1);
#endif
  }

  value_type top() const {
    assert(size() > 0);
    return array_[0];
  }

  value_type pop() {
    value_type a = top();
#ifdef STD_POP_HEAP
    std::pop_heap(array_.begin(), array_.end(), cmp_);
    array_.pop_back();
#else
    value_type b = array_.back();
    array_.pop_back();
    if (size() > 0) {
      push_down(b, 0);
    }
#endif
    return a;
  }

  void sort() {
    for (size_type i = size(); i > 0; --i) array_[i - 1] = pop();
  }

  bool is_sorted(size_type sz) const {
    return std::is_sorted(array_.begin(), array_.begin() + sz, cmp_);
  }

  void clear() {
    array_.clear();
    array_.shrink_to_fit(); // to match heap_clear(heap*)
  }

 private:
  static size_type parent(size_type q) { return (q - 1) / 2; }
  static size_type children(size_type p) { return (p * 2) + 1; }
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  Compare cmp_;
  array_type array_;
};

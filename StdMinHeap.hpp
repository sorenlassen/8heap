#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <new>
#include <vector>

class StdMinHeap {
  typedef std::vector<value_type> array_type;
 public:
  typedef uint16_t value_type;
  typedef array_type::size_type size_type;

  StdMinHeap() { }
  ~StdMinHeap() = default;
  StdMinHeap(const StdMinHeap&) = delete;
  StdMinHeap& operator=(const StdMinHeap&) = delete;

  size_type size() const { return array_.size(); }
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
  void pull_up(value_type b, size_type q) { }
  void push_down(value_type a, size_type p) { }
  void heapify() {
    std::make_heap(array_.begin(), array_.end(), std::greater<value_type>());
  }
  bool is_heap() const {
    return std::is_heap(array_.begin(), array_.end(), std::greater<value_type>());
  }
  bool push(value_type b) {
    array_.push_back(b);
    std::push_heap(array_.begin(), array_.end(), std::greater<value_type>());
    return true;
  }
  value_type top() const {
    assert(size() > 0);
    return array_[0];
  }
  value_type pop() {
    value_type a = top();
    std::pop_heap(array_.begin(), array_.end(), std::greater<value_type>());
    array_.pop_back();
    return a;
  }
  void sort() {
    for (size_type i = size(); i > 0; --i) array_[i - 1] = pop();
  }
  void clear() {
    array_.clear();
    array_.shrink_to_fit(); // to match heap_clear(heap*)
  }

 private:
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  array_type array_;
};

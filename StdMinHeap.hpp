#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <vector>

class StdMinHeap {
 public:
  typedef uint16_t value_type;
  typedef std::size_t size_type;

  StdMinHeap() { }
  ~StdMinHeap() = default;
  StdMinHeap(const StdMinHeap&) = delete;
  StdMinHeap& operator=(const StdMinHeap&) = delete;

  size_type size() const { return array_.size(); }
  value_type* extend(size_type n) {
    size_type old_size = array_.size();
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
  void clear() {
    array_.clear();
    array_.shrink_to_fit(); // to match heap_clear(heap*)
  }

 private:
  std::vector<value_type> array_;
};

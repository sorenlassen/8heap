#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <vector>

class StdMinHeap {
 public:
  typedef uint16_t elem_type;
  typedef std::size_t size_type;

  StdMinHeap() { }
  ~StdMinHeap() { }
  inline size_type size() const { return array.size(); }
  inline elem_type* extend(size_t n) {
    size_t old_size = array.size();
    array.resize(old_size + n);
    return &array[old_size];
  }
  template<class InputIterator>
  inline void append(InputIterator begin, InputIterator end) {
    array.insert(array.end(), begin, end);
  }
  inline void pull_up(elem_type b, size_t q) { }
  inline void push_down(elem_type a, size_t p) { }
  inline void heapify() {
    std::make_heap(array.begin(), array.end(), std::greater<elem_type>());
  }
  inline bool push(elem_type b) {
    array.push_back(b);
    std::push_heap(array.begin(), array.end(), std::greater<elem_type>());
    return true;
  }
  inline elem_type top() const {
    assert(size() > 0);
    return array[0];
  }
  inline elem_type pop() {
    elem_type a = top();
    std::pop_heap(array.begin(), array.end(), std::greater<elem_type>());
    array.pop_back();
    return a;
  }
  inline void clear() {
    std::vector<elem_type> empty;
    array.swap(empty);
  }

 private:
  std::vector<elem_type> array;
};

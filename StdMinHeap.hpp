#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <memory>

class StdMinHeap {
 public:
  typedef uint16_t elem_type;
  typedef std::size_t size_type;

  StdMinHeap() : capacity(0), size(0) { }
  ~StdMinHeap() { }
  inline elem_type* extend(size_t n) {
    size_t new_size = size + n;
    if (new_size > capacity) {
      size_t new_capacity = align_up(std::max(2 * capacity, new_size));
      // TODO(soren): test if it's faster to use malloc() instead and change
      // array and new_array to use free() as deleter
      // (new[] needlessly initializes the memory it allocates).
      std::unique_ptr<elem_type[]> new_array(new elem_type[new_capacity]);
      std::move(array.get(), array.get() + size, new_array.get());
      array = std::move(new_array);
      capacity = new_capacity;
    }
    size = new_size;
    return array.get() + new_size - n;
  }
  inline void pull_up(elem_type b, size_t q) { }
  inline void push_down(elem_type a, size_t p) { }
  inline void heapify() {
    std::make_heap(array.get(), array.get() + size, std::greater<elem_type>());
  }
  inline bool push(elem_type b) {
    *extend(1) = b;
    std::push_heap(array.get(), array.get() + size, std::greater<elem_type>());
    return true;
  }
  inline elem_type top() const {
    assert(size > 0);
    return array[0];
  }
  inline elem_type pop() {
    assert(size > 0);
    std::pop_heap(array.get(), array.get() + size, std::greater<elem_type>());
    size--;
    return array.get()[size];
  }

 private:
  static constexpr size_t align = 8; // capacity is always a multiple of this
  static inline size_t align_up(size_t n) { return (n + align - 1) & ~(align - 1); }

  std::unique_ptr<elem_type[]> array;
  size_t capacity;
  size_t size;
};

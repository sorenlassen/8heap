#pragma once

extern "C" {
#include "h8.h"
}
#include <new>
#include <algorithm>
#include <iterator>

class H8 {
 public:
  typedef uint16_t value_type;
  typedef ::size_t size_type;

  H8() { heap_init(&h_); }
  ~H8() { heap_clear(&h_); }
  H8(const H8&) = delete;
  H8& operator=(const H8&) = delete;

  size_type size() const { return h_.size; }
  value_type& operator[](size_type index) {
    return h_.array[index];
  }
  value_type* extend(size_type n) {
    value_type* ptr = heap_extend(&h_, n);
    if (!ptr) throw_bad_alloc();
    return ptr;
  }
  template<class InputIterator>
  void append(InputIterator begin, InputIterator end) {
    value_type* ptr = heap_extend(&h_, std::distance(begin, end));
    std::copy(begin, end, ptr);
  }
  void pull_up(value_type b, size_type q) { heap_pull_up(&h_, b, q); }
  void push_down(value_type a, size_type p) { heap_push_down(&h_, a, p); }
  void heapify() { heap_heapify(&h_); }
  void push(value_type b) {
    bool ok = heap_push(&h_, b);
    if (!ok) throw_bad_alloc();
  }
  bool is_heap() const { return heap_is_heap(&h_); }
  value_type top() const { return heap_top(&h_); }
  value_type pop() { return heap_pop(&h_); }
  void sort() { heap_sort(&h_); }
  bool is_sorted(size_type sz) const {
    return std::is_sorted(h_.array, h_.array + sz, std::greater<value_type>());
  }
  void clear() { heap_clear(&h_); }

 private:
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  heap h_;
};

#pragma once

extern "C" {
#include "h8.h"
}

class H8 {
 public:
  typedef uint16_t elem_type;
  typedef ::size_t size_type;

  H8() { heap_init(&h); }
  ~H8() { heap_clear(&h); }
  inline size_type size() const { return h.size; }
  inline elem_type* extend(size_type n) { return heap_extend(&h, n); }
  inline void pull_up(elem_type b, size_type q) { heap_pull_up(&h, b, q); }
  inline void push_down(elem_type a, size_type p) { heap_push_down(&h, a, p); }
  inline void heapify() { heap_heapify(&h); }
  inline bool push(elem_type b) { return heap_push(&h, b); }
  inline elem_type top() const { return heap_top(&h); }
  inline elem_type pop() { return heap_pop(&h); }

 private:
  heap h;
};

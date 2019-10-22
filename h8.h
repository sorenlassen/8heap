#pragma once

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // uint16_t

typedef uint16_t value_type;

typedef struct {
  // The empty heap is represented by NULL if capacity is zero,
  // otherwise heap points to a memory block of size capacity.
  value_type* array;
  size_t capacity;
  size_t size;
} heap;

extern void heap_init(heap* h);

extern void heap_clear(heap* h);

// Increases heap by n consecutive value positions at the end and returns a
// pointer to the first of those positions.
//
// Note that this function breaks the heap invariant. After calling this
// function the caller must populate the new n positions at the end of the
// heap array and then call heapify on those n positions.
//
// Returns NULL if memory allocation fails,
extern value_type* heap_extend(heap* h, size_t n);

extern void heap_pull_up(heap* h, value_type b, size_t q);

extern void heap_push_down(heap* h, value_type a, size_t p);

extern void heap_heapify(heap* h);

extern bool heap_is_heap(heap const* h);

extern bool heap_push(heap* h, value_type b);

extern value_type heap_top(heap const* h);

extern value_type heap_pop(heap* h);

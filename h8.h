#pragma once

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // uint16_t, SIZE_MAX

typedef uint16_t h8_value_type;

#define H8_ARITY 8
#define H8_SIZE_MAX ((SIZE_MAX / sizeof(h8_value_type)) & ~(H8_ARITY - 1))

typedef struct {
  // The empty heap is represented by NULL if capacity is zero,
  // otherwise heap points to a memory block of size capacity.
  h8_value_type* array;
  size_t capacity;
  size_t size;
} h8_heap;

#ifdef __cplusplus
extern "C" {
#endif

void h8_heap_init(h8_heap* h);

void h8_heap_clear(h8_heap* h);

// Increases heap by n consecutive value positions at the end and returns a
// pointer to the first of those positions.
//
// Note that this function breaks the heap invariant. After calling this
// function the caller must populate the new n positions at the end of the
// heap array and then call heapify on those n positions.
//
// Returns NULL if memory allocation fails or h->size + n > H8_SIZE_MAX.
h8_value_type* h8_heap_extend(h8_heap* h, size_t n);

void h8_heap_pull_up(h8_heap* h, h8_value_type b, size_t q);

void h8_heap_push_down(h8_heap* h, h8_value_type a, size_t p);

void h8_heap_heapify(h8_heap* h);

// Returns true if h->array points to h->size values that satisfy
// the min-heap invariant for arity H8_ARITY.
bool h8_heap_is_heap(h8_heap const* h);

// Adds b to heap and adjusts it to maintain the heap invariant.
// Precondition: h8_heap_is_heap(h).
// Returns false is memory allocation fails or h->size == H8_SIZE_MAX.
bool h8_heap_push(h8_heap* h, h8_value_type b);

h8_value_type h8_heap_top(h8_heap const* h);

h8_value_type h8_heap_pop(h8_heap* h);

// Precondition: h8_heap_is_heap(h).
// Postcondition: h->array[0,h-size) is sorted in descending order.
void h8_heap_sort(h8_heap* h);

#ifdef __cplusplus
}
#endif

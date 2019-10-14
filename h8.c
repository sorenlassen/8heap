/*
   gcc -g -std=c11 -msse4 -c h8.c # optimize with -O2 -DNDEBUG
*/

#include "h8.h"

#include <assert.h> // assert, static_assert
#include <stdalign.h> // alignof
#include <stdarg.h> // va_list, va_start, va_end
#include <stdbool.h> // bool
#include <stddef.h> // size_t, NULL
#include <stdint.h> // uint16_t, UINT16_MAX
#include <stdnoreturn.h> // noreturn
#include <stdlib.h> // exit
#include <string.h> // memcpy

#include <emmintrin.h> // __m128i
#include <immintrin.h>
#include <smmintrin.h>

//// Generic helper functions: ////

// Similar to boost/align but for number, not pointer.
static bool is_aligned(size_t n, size_t alignment) {
  assert(alignment > 0);
  assert((alignment & (alignment - 1)) == 0); // is power of 2
  return (n & (alignment - 1)) == 0;
}

// Similar to boost/align but for number, not pointer.
static size_t align_down(size_t n, size_t alignment) {
  assert(alignment > 0);
  assert((alignment & (alignment - 1)) == 0); // is power of 2
  return n & ~(alignment - 1);
}

// Similar to boost/align but for number, not pointer.
static size_t align_up(size_t n, size_t alignment) {
  return align_down(n + alignment - 1, alignment);
}

// C11 aligned_alloc is not available on mac.
static void* aligned_alloc(size_t alignment, size_t sz) {
  void* ptr = NULL;
  return 0 == posix_memalign(&ptr, alignment, sz) ? ptr : NULL;
}

//// Heap types: ////

// 8-ary min heap with element type unsigned 16 bit integers.
#define ELEM_MAX UINT16_MAX;
#define ARITY 8
#define ALIGN 16 // ARITY * sizeof(elem)

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef elem elem_vector __attribute__ ((vector_size (ALIGN)));
typedef union {
  __m128i mm;
  elem_vector elems;
} v128;

// From https://en.wikibooks.org/wiki/C_Programming/Preprocessor_directives_and_macros
#define num2str(x) str(x)
#define str(x) #x

static_assert(alignof(v128) == ALIGN, "v128 alignment should be " num2str(ALIGN));
static_assert(sizeof(v128) == ARITY * sizeof(elem), num2str(ARITY) " elements should fill up v128");

#undef str
#undef num2str

// The following is gcc specific, see: https://stackoverflow.com/a/35268748
// whereas _mm_set1_epi16(ELEM_MAX) would be more cross platform
static const v128 v128_max = { { ~0LL, ~0LL } };

//// Heap types helper functions: ////

typedef int minpos_type;
static minpos_type minpos(v128 v) {
  return _mm_cvtsi128_si32(_mm_minpos_epu16(v.mm));
}
static elem minpos_min(minpos_type x) { return (uint16_t)x; }
static size_t minpos_pos(minpos_type x) { return x >> 16; }

//// Heap state: ////

// The empty heap is represented by NULL if capacity is zero,
// otherwise heap points to a memory block of size capacity.
static elem* heap = NULL;
size_t capacity = 0;
size_t padded = 0;
size_t size = 0;

//// Private functions: ////

static v128* heap_vector(size_t p) {
  assert(is_aligned(p, ARITY));
  return (v128*)(heap + p);
}

//// Public functions: ////

void clear() {
  free(heap);
  heap = NULL;
  capacity = 0;
  padded = 0;
  size = 0;
}

size_t heap_size() { return size; }

// Increases heap by n consecutive element positions at the end and returns a
// pointer to the first of those positions.
//
// Note that this function breaks the heap invariant. After calling this
// function the caller must populate the new n positions at the end of the
// heap array and then call heapify on those n positions.
//
// Returns NULL if memory allocation fails,
elem* extend_heap(size_t n) {
  size_t new_size = size + n;

  if (new_size > padded) {
    if (new_size > capacity) {
      size_t new_capacity = capacity == 0 ? ARITY : (2 * capacity);
      if (new_capacity < new_size) {
        new_capacity = align_up(new_size, ARITY);
      }
      elem* new_heap = (elem*)aligned_alloc(ALIGN, new_capacity * sizeof(elem));
      if (!new_heap) {
        return NULL;
      }
      // TODO(soren): Measure if it's faster to utilize that we copy an integral
      // number of aligned v128s, e.g. with SSE instructions.
      memcpy(new_heap, heap, align_up(size, ARITY) * sizeof(elem));
      free(heap);
      heap = new_heap;
      capacity = new_capacity;
    }
    padded = align_up(new_size, ARITY);
    if (padded >= size + ARITY) {
      // Unnecessary if padded == new_size but we just do it always.
      heap_vector(padded)[-1] = v128_max;
    }
  }

  size = new_size;
  return heap + new_size - n;
}

void pull_up(elem b, size_t q) {
  assert(q < size);
  while (q >= ARITY) {
    size_t p = (q / ARITY) - 1;
    elem a = heap[p];
    if (a <= b) break;
    heap[q] = a;
    q = p;
  }
  heap[q] = b;
}

void push_down(elem a, size_t p) {
  assert(p < size);
  while (true) {
    size_t q = (p + 1) * ARITY;
    if (q >= size) break;
    minpos_type x = minpos(*heap_vector(q));
    elem b = minpos_min(x);
    if (a <= b) break;
    heap[p] = b;
    p = q + minpos_pos(x);
  }
  heap[p] = a;
}

void heapify(size_t skip) {
  if (skip < ARITY) skip = ARITY;
  if (skip >= size) return;

  size_t q = align_down(size - 1, ARITY);
  do {
    minpos_type x = minpos(*heap_vector(q));
    elem b = minpos_min(x);
    size_t p = (q / ARITY) - 1;
    elem a = heap[p];
    if (b < a) {
      heap[p] = b;
      push_down(a, q + minpos_pos(x));
    }
    q -= 8;
  } while (q > 0);
}

bool push(elem b) {
  if (!extend_heap(1)) {
    return false;
  }
  pull_up(b, size - 1);
  return true;
}

elem top() {
  assert(size > 0);
  minpos_type x = minpos(*heap_vector(0));
  return minpos_min(x);
}

elem pop() {
  assert(size > 0);
  minpos_type x = minpos(*heap_vector(0));
  elem b = minpos_min(x);
  elem a = heap[size - 1];
  heap[size - 1] = ELEM_MAX;
  size--;
  size_t p = minpos_pos(x);
  if (p != size) {
    push_down(a, p);
  }
  return b;
}

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

// 8-ary min heap with value type unsigned 16 bit integers.
#define VALUE_MAX UINT16_MAX;
#define ARITY 8
#define ALIGN 16 // ARITY * sizeof(value_type)

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef value_type value_vector __attribute__ ((vector_size (ALIGN)));
typedef union {
  __m128i mm;
  value_vector values;
} v128;

// From https://en.wikibooks.org/wiki/C_Programming/Preprocessor_directives_and_macros
#define num2str(x) str(x)
#define str(x) #x

static_assert(alignof(v128) == ALIGN,
              "v128 alignment should be " num2str(ALIGN));
static_assert(sizeof(v128) == ARITY * sizeof(value_type),
              num2str(ARITY) " values should fill up v128");

#undef str
#undef num2str

// The following is gcc specific, see: https://stackoverflow.com/a/35268748
// whereas _mm_set1_epi16(VALUE_MAX) would be more cross platform
static const v128 v128_max = { { ~0LL, ~0LL } };

//// Heap types helper functions: ////

typedef int minpos_type;
static minpos_type minpos(v128 v) {
  return _mm_cvtsi128_si32(_mm_minpos_epu16(v.mm));
}
static value_type minpos_min(minpos_type x) { return (uint16_t)x; }
static size_t minpos_pos(minpos_type x) { return x >> 16; }

//// Private functions: ////

static size_t parent(size_t q) { return (q / ARITY) - 1; }

static size_t children(size_t p) { return (p + 1) * ARITY; }

static void heap_vector_set(heap* h, size_t p, v128 v) {
  assert(is_aligned(p, ARITY));
  assert(p + 8 <= h->capacity);
  *(v128*)(h->array + p) = v;
}

static minpos_type heap_vector_minpos(heap const* h, size_t p) {
  assert(is_aligned(p, ARITY));
  assert(p < h->size);
  return minpos(*(v128 const*)(h->array + p));
}

//// Public functions: ////

void heap_init(heap* h) {
  h->array = NULL;
  h->capacity = 0;
  h->size = 0;
}

void heap_clear(heap* h) {
  free(h->array);
  heap_init(h);
}

value_type* heap_extend(heap* h, size_t n) {
  size_t new_size = h->size + n;

  size_t padded_size = align_up(h->size, ARITY);
  if (new_size > padded_size) {
    size_t padded_new_size = align_up(new_size, ARITY);
    if (new_size > h->capacity) {
      size_t new_capacity = h->capacity == 0 ? ARITY : (2 * h->capacity);
      if (new_capacity < padded_new_size) {
        new_capacity = padded_new_size;
      }
      value_type* new_array =
          (value_type*)aligned_alloc(ALIGN, new_capacity * sizeof(value_type));
      if (!new_array) {
        return NULL;
      }
      // TODO(soren): Measure if it's faster to utilize that we copy an integral
      // number of aligned v128s, e.g. with SSE instructions.
      memcpy(new_array, h->array, padded_size * sizeof(value_type));
      free(h->array);
      h->array = new_array;
      h->capacity = new_capacity;
    }
    // Unnecessary if new_size == padded_new_size but we just do it always.
    heap_vector_set(h, padded_new_size - ARITY, v128_max);
  }

  h->size = new_size;
  return h->array + new_size - n;
}

void heap_pull_up(heap* h, value_type b, size_t q) {
  assert(q < h->size);
  while (q >= ARITY) {
    size_t p = parent(q);
    value_type a = h->array[p];
    if (a <= b) break;
    h->array[q] = a;
    q = p;
  }
  h->array[q] = b;
}

void heap_push_down(heap* h, value_type a, size_t p) {
  assert(p < h->size);
  while (true) {
    size_t q = children(p);
    if (q >= h->size) break;
    minpos_type x = heap_vector_minpos(h, q);
    value_type b = minpos_min(x);
    if (a <= b) break;
    h->array[p] = b;
    p = q + minpos_pos(x);
  }
  h->array[p] = a;
}

void heap_heapify(heap* h) {
  if (h->size <= ARITY) return;

  size_t q = align_down(h->size - 1, ARITY);

  // The first while loop is an optimization for the bottom level of the heap,
  // inlining the call to heap_push_down which is trivial at the bottom level.
  // Here "bottom level" means the 8-vectors without children.
  size_t r = parent(q);
  while (q > r) {
    minpos_type x = heap_vector_minpos(h, q);
    value_type b = minpos_min(x);
    size_t p = parent(q);
    value_type a = h->array[p];
    if (b < a) {
      h->array[p] = b;
      // The next line inlines heap_push_down(h, a, q + minpos_pos(x))
      // with the knowledge that children(q) >= h->size.
      h->array[q + minpos_pos(x)] = a;
    }
    q -= ARITY;
  }

  while (q > 0) {
    minpos_type x = heap_vector_minpos(h, q);
    value_type b = minpos_min(x);
    size_t p = parent(q);
    value_type a = h->array[p];
    if (b < a) {
      h->array[p] = b;
      heap_push_down(h, a, q + minpos_pos(x));
    }
    q -= ARITY;
  }
}

bool heap_is_heap(heap const* h) {
  if (h->size <= ARITY) return true;
  size_t q = align_down(h->size - 1, ARITY);
  while (q > 0) {
    minpos_type x = heap_vector_minpos(h, q);
    value_type b = minpos_min(x);
    size_t p = parent(q);
    value_type a = h->array[p];
    if (b < a) return false;
    q -= ARITY;
  }
  return true;
}

bool heap_push(heap* h, value_type b) {
  if (!heap_extend(h, 1)) {
    return false;
  }
  heap_pull_up(h, b, h->size - 1);
  return true;
}

value_type heap_top(heap const* h) {
  assert(h->size > 0);
  minpos_type x = heap_vector_minpos(h, 0);
  return minpos_min(x);
}

value_type heap_pop(heap* h) {
  assert(h->size > 0);
  minpos_type x = heap_vector_minpos(h, 0);
  value_type b = minpos_min(x);
  value_type a = h->array[h->size - 1];
  h->array[h->size - 1] = VALUE_MAX;
  h->size--;
  size_t p = minpos_pos(x);
  if (p != h->size) {
    heap_push_down(h, a, p);
  }
  return b;
}

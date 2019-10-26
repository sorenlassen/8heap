/*
   gcc -g -std=c11 -msse4 -c h8.c # optimize with -O2 -DNDEBUG
*/

#include "h8.h"
#include "minpos.h"
#include <assert.h> // assert, static_assert
#include <stdalign.h> // alignof
#include <stdarg.h> // va_list, va_start, va_end
#include <stdbool.h> // bool
#include <stddef.h> // size_t, NULL
#include <stdint.h> // uint16_t, UINT16_MAX
#include <stdnoreturn.h> // noreturn
#include <stdlib.h> // exit
#include <string.h> // memcpy

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
#define VALUE_MAX UINT16_MAX
#define ALIGN 16 // H8_ARITY * sizeof(value_type)

// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef value_type value_vector __attribute__ ((vector_size (ALIGN)));
typedef union {
  value_vector values;
  __m128i mm;
} v128;

// From https://en.wikibooks.org/wiki/C_Programming/Preprocessor_directives_and_macros
#define num2str(x) str(x)
#define str(x) #x

static_assert(H8_ARITY <= H8_SIZE_MAX,
              "arity " num2str(H8_ARITY) " may not exceed H8_SIZE_MAX");
static_assert(H8_SIZE_MAX % H8_ARITY == 0,
              "H8_SIZE_MAX must be a multiple of arity " num2str(H8_ARITY));
static_assert(H8_SIZE_MAX <= SIZE_MAX / sizeof(value_type),
              "H8_SIZE_MAX * sizeof(value_type) may not exceed SIZE_MAX");
static_assert(alignof(v128) == ALIGN,
              "v128 alignment should be " num2str(ALIGN));
static_assert(sizeof(v128) == H8_ARITY * sizeof(value_type),
              num2str(H8_ARITY) " values should fill up v128");

#undef str
#undef num2str

static const v128 v128_max = { {
  VALUE_MAX, VALUE_MAX, VALUE_MAX, VALUE_MAX,
  VALUE_MAX, VALUE_MAX, VALUE_MAX, VALUE_MAX,
} };

//// Private functions: ////

static size_t parent(size_t q) { return (q / H8_ARITY) - 1; }

static size_t children(size_t p) { return (p + 1) * H8_ARITY; }

static void heap_vector_set(heap* h, size_t p, v128 v) {
  assert(is_aligned(p, H8_ARITY));
  assert(p + 8 <= h->capacity);
  *(v128*)(h->array + p) = v;
}

static minpos_type heap_vector_minpos(heap const* h, size_t p) {
  assert(is_aligned(p, H8_ARITY));
  assert(p < h->size);
  return minpos(((v128 const*)(h->array + p))->mm);
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
  if (n >= H8_SIZE_MAX - h->size) return NULL;
  size_t new_size = h->size + n;
  // padded_size and padded_new_size below are <= H8_SIZE_MAX because h->size
  // and new_size are and because H8_SIZE_MAX is a multiple of H8_ARITY.
  size_t padded_size = align_up(h->size, H8_ARITY);
  if (new_size > padded_size) {
    size_t padded_new_size = align_up(new_size, H8_ARITY);
    if (new_size > h->capacity) {
      // Guaranteed to not exceed H8_SIZE_MAX because H8_ARITY <= H8_SIZE_MAX.
      size_t new_capacity =
        h->capacity == 0
        ? H8_ARITY
        : h->capacity <= H8_SIZE_MAX / 2
          ? 2 * h->capacity
          : H8_SIZE_MAX;
      if (new_capacity < padded_new_size) {
        new_capacity = padded_new_size;
      }
      // Guaranteed to not overflow/wrap-around or exceed SIZE_MAX because
      // new_capacity <= H8_SIZE_MAX <= SIZE_MAX / sizeof(value_type).
      size_t num_bytes = new_capacity * sizeof(value_type);
      value_type* new_array = (value_type*)aligned_alloc(ALIGN, num_bytes);
      if (!new_array) return NULL;
      // TODO(soren): Measure if it's faster to utilize that we copy an integral
      // number of aligned v128s, e.g. with SSE instructions.
      memcpy(new_array, h->array, padded_size * sizeof(value_type));
      free(h->array);
      h->array = new_array;
      h->capacity = new_capacity;
    }
    // Unnecessary if new_size == padded_new_size but we just do it always.
    heap_vector_set(h, padded_new_size - H8_ARITY, v128_max);
  }
  h->size = new_size;
  return h->array + new_size - n;
}

void heap_pull_up(heap* h, value_type b, size_t q) {
  assert(q < h->size);
  while (q >= H8_ARITY) {
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
  if (h->size <= H8_ARITY) return;

  size_t q = align_down(h->size - 1, H8_ARITY);

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
    q -= H8_ARITY;
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
    q -= H8_ARITY;
  }
}

bool heap_is_heap(heap const* h) {
  if (h->size <= H8_ARITY) return true;
  size_t q = align_down(h->size - 1, H8_ARITY);
  while (q > 0) {
    minpos_type x = heap_vector_minpos(h, q);
    value_type b = minpos_min(x);
    size_t p = parent(q);
    value_type a = h->array[p];
    if (b < a) return false;
    q -= H8_ARITY;
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

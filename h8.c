/*
   gcc -g -std=c11 -msse4 -c h8.c # optimize with -O2 -DNDEBUG
*/

#include "h8.h"
#include "minpos.h"
#include "v128.h"
#include "align.h"
#include <assert.h> // assert, static_assert
#include <stdalign.h> // alignof
#include <stdarg.h> // va_list, va_start, va_end
#include <stdbool.h> // bool
#include <stddef.h> // size_t, NULL
#include <stdint.h> // uint16_t, UINT16_MAX
#include <stdnoreturn.h> // noreturn
#include <stdlib.h> // exit
#include <string.h> // memcpy

//// Private constants: ////

#define VALUE_MAX UINT16_MAX
#define ALIGN 16 // H8_ARITY * sizeof(h8_value_type)

// From https://en.wikibooks.org/wiki/C_Programming/Preprocessor_directives_and_macros
#define num2str(x) str(x)
#define str(x) #x

static_assert(H8_ARITY <= H8_SIZE_MAX,
              "arity " num2str(H8_ARITY) " may not exceed H8_SIZE_MAX");
static_assert(H8_SIZE_MAX % H8_ARITY == 0,
              "H8_SIZE_MAX must be a multiple of arity " num2str(H8_ARITY));
static_assert(H8_SIZE_MAX <= SIZE_MAX / sizeof(h8_value_type),
              "H8_SIZE_MAX * sizeof(h8_value_type) may not exceed SIZE_MAX");
static_assert(alignof(v128) == ALIGN,
              "v128 alignment should be " num2str(ALIGN));
static_assert(sizeof(v128) == H8_ARITY * sizeof(h8_value_type),
              num2str(H8_ARITY) " values should fill up v128");

#undef str
#undef num2str

//// Private functions: ////

static size_t parent(size_t q) { return (q / H8_ARITY) - 1; }

static size_t children(size_t p) { return (p + 1) * H8_ARITY; }

static void heap_vector_set(h8_heap* h, size_t p, v128 v) {
  assert(is_aligned(p, H8_ARITY));
  assert(p + 8 <= h->capacity);
  *(v128*)(h->array + p) = v;
}

static minpos_type heap_vector_minpos(h8_heap const* h, size_t p) {
  assert(is_aligned(p, H8_ARITY));
  assert(p < h->size);
  return minpos8(h->array + p);
}

//// Public functions: ////

void h8_heap_init(h8_heap* h) {
  h->array = NULL;
  h->capacity = 0;
  h->size = 0;
}

void h8_heap_clear(h8_heap* h) {
  free(h->array);
  h8_heap_init(h);
}

h8_value_type* h8_heap_extend(h8_heap* h, size_t n) {
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
      // new_capacity <= H8_SIZE_MAX <= SIZE_MAX / sizeof(h8_value_type).
      size_t num_bytes = new_capacity * sizeof(h8_value_type);
      h8_value_type* new_array = (h8_value_type*)aligned_alloc(ALIGN, num_bytes);
      if (!new_array) return NULL;
      // TODO(soren): Measure if it's faster to utilize that we copy an integral
      // number of aligned v128s, e.g. with SSE instructions.
      memcpy(new_array, h->array, padded_size * sizeof(h8_value_type));
      free(h->array);
      h->array = new_array;
      h->capacity = new_capacity;
    }
    // Unnecessary if new_size == padded_new_size but we just do it always.
    heap_vector_set(h, padded_new_size - H8_ARITY, kV128Max);
  }
  h->size = new_size;
  return h->array + new_size - n;
}

void h8_heap_pull_up(h8_heap* h, h8_value_type b, size_t q) {
  assert(q < h->size);
  while (q >= H8_ARITY) {
    size_t p = parent(q);
    h8_value_type a = h->array[p];
    if (a <= b) break;
    h->array[q] = a;
    q = p;
  }
  h->array[q] = b;
}

void h8_heap_push_down(h8_heap* h, h8_value_type a, size_t p) {
  assert(p < h->size);
  while (true) {
    size_t q = children(p);
    if (q >= h->size) break;
    minpos_type x = heap_vector_minpos(h, q);
    h8_value_type b = minpos_min(x);
    if (a <= b) break;
    h->array[p] = b;
    p = q + minpos_pos(x);
  }
  h->array[p] = a;
}

void h8_heap_heapify(h8_heap* h) {
  if (h->size <= H8_ARITY) return;

  size_t q = align_down(h->size - 1, H8_ARITY);

  // The first while loop is an optimization for the bottom level of the heap,
  // inlining the call to heap_push_down which is trivial at the bottom level.
  // Here "bottom level" means the 8-vectors without children.
  size_t r = parent(q);
  while (q > r) {
    minpos_type x = heap_vector_minpos(h, q);
    h8_value_type b = minpos_min(x);
    size_t p = parent(q);
    h8_value_type a = h->array[p];
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
    h8_value_type b = minpos_min(x);
    size_t p = parent(q);
    h8_value_type a = h->array[p];
    if (b < a) {
      h->array[p] = b;
      h8_heap_push_down(h, a, q + minpos_pos(x));
    }
    q -= H8_ARITY;
  }
}

bool h8_heap_is_heap(h8_heap const* h) {
  if (h->size <= H8_ARITY) return true;
  size_t q = align_down(h->size - 1, H8_ARITY);
  while (q > 0) {
    minpos_type x = heap_vector_minpos(h, q);
    h8_value_type b = minpos_min(x);
    size_t p = parent(q);
    h8_value_type a = h->array[p];
    if (b < a) return false;
    q -= H8_ARITY;
  }
  return true;
}

bool h8_heap_push(h8_heap* h, h8_value_type b) {
  if (!h8_heap_extend(h, 1)) {
    return false;
  }
  h8_heap_pull_up(h, b, h->size - 1);
  return true;
}

h8_value_type h8_heap_top(h8_heap const* h) {
  assert(h->size > 0);
  minpos_type x = heap_vector_minpos(h, 0);
  return minpos_min(x);
}

h8_value_type h8_heap_pop(h8_heap* h) {
  assert(h->size > 0);
  minpos_type x = heap_vector_minpos(h, 0);
  h8_value_type b = minpos_min(x);
  h8_value_type a = h->array[h->size - 1];
  h->array[h->size - 1] = VALUE_MAX;
  h->size--;
  size_t p = minpos_pos(x);
  if (p != h->size) {
    h8_heap_push_down(h, a, p);
  }
  return b;
}

void h8_heap_sort(h8_heap* h) {
  v128 v = kV128Max;
  size_t x = h->size;
  size_t i = x % H8_ARITY;
  x -= i;
  if (i != 0) {
    do {
      --i;
      v.values[i] = h8_heap_pop(h);
    } while (i > 0);
    heap_vector_set(h, x, v);
  }
  while (x > 0) {
    x -= H8_ARITY;
    for (size_t j = H8_ARITY; j > 0; --j) {
      v.values[j - 1] = h8_heap_pop(h);
    }
    heap_vector_set(h, x, v);
  }
}

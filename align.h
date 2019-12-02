#pragma once

#include <stddef.h>
#include <stdlib.h> // aligned_alloc, posix_memalign

#if defined(__clang__) && __clang_major__ < 11
// C11 aligned_alloc is not available on mac.
static void* aligned_alloc(size_t alignment, size_t sz) {
  void* ptr = NULL;
  return 0 == posix_memalign(&ptr, alignment, sz) ? ptr : NULL;
}
#endif

#ifdef __cplusplus
#define ALIGN_CONSTEXPR constexpr
#else
#define ALIGN_CONSTEXPR
#endif

// Similar to boost/align but for number, not pointer.
static inline bool ALIGN_CONSTEXPR is_aligned(size_t n, size_t alignment) {
  return n % alignment == 0;
}

// Similar to boost/align but for number, not pointer.
static inline size_t ALIGN_CONSTEXPR align_down(size_t n, size_t alignment) {
  return n - (n % alignment);
}

// Similar to boost/align but for number, not pointer.
static inline size_t ALIGN_CONSTEXPR align_up(size_t n, size_t alignment) {
  size_t m = n % alignment;
  return n + (m ? alignment - m : 0);
}

#undef ALIGN_CONSTEXPR

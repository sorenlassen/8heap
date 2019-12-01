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

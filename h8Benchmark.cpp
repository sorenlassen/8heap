/*
   brew install folly
   gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c
   g++ -g -std=c++14 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o h8Benchmark.cpp
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
extern "C" {
#include "h8.h"
}
#include <folly/Benchmark.h>

using namespace folly;

void fill(elem_type* ptr, size_t sz, bool sorted) {
  double f = 65536.0 / sz;
  for (size_t i = 0; i < sz; ++i) {
    ptr[i] = (elem_type)((sorted ? i : (sz - 1 - i)) * f);
  }
}

void heapify_h8(uint32_t n, size_t sz, bool sorted) {
  elem_type x = 0;
  heap h;
  heap_init(&h);
  elem_type* ptr = heap_extend(&h, sz);
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(ptr, sz, sorted);
    }
    heap_heapify(&h, 0);
    x ^= heap_top(&h);
  }
  doNotOptimizeAway(x);
  heap_clear(&h);
}

void heapify_std(uint32_t n, size_t sz, bool sorted) {
  elem_type x = 0;
  elem_type* ptr = (elem_type*)malloc(sz * sizeof(elem_type));
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(ptr, sz, sorted);
    }
    std::make_heap(ptr, ptr + sz);
    x ^= ptr[0];
  }
  doNotOptimizeAway(x);
  free(ptr);
}

void heapify_h8_sorted(uint32_t n, size_t sz) { heapify_h8(n, sz, true); }
void heapify_h8_unsorted(uint32_t n, size_t sz) { heapify_h8(n, sz, false); }
void heapify_std_sorted(uint32_t n, size_t sz) { heapify_std(n, sz, true); }
void heapify_std_unsorted(uint32_t n, size_t sz) { heapify_std(n, sz, false); }
BENCHMARK_PARAM(heapify_h8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 1000)
BENCHMARK_PARAM(heapify_h8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 100000)
BENCHMARK_PARAM(heapify_h8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 1000)
BENCHMARK_PARAM(heapify_h8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 100000)

int main(int argc, char** argv) {
  runBenchmarks();
  return 0;
}

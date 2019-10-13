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

void fill(elem* ptr, size_t sz, bool sorted) {
  double f = 65536.0 / sz;
  for (size_t i = 0; i < sz; ++i) {
    ptr[i] = (elem)((sorted ? i : (sz - 1 - i)) * f);
  }
}

void do_heapify(uint32_t n, size_t sz, bool sorted, bool std) {
  elem x = 0;
  elem* ptr = std ? (elem*)malloc(sz * sizeof(elem)) : extend_heap(sz);
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(ptr, sz, sorted);
    }
    if (std) {
      std::make_heap(ptr, ptr + sz);
      x ^= ptr[0];
    } else {
      heapify(0);
      x ^= top();
    }
  }
  doNotOptimizeAway(x);
  if (std) {
    free(ptr);
  } else {
    clear();
  }
}
void heapify_sorted(uint32_t n, size_t sz) { do_heapify(n, sz, true, false); }
void heapify_unsorted(uint32_t n, size_t sz) { do_heapify(n, sz, false, false); }
void heapify_sorted_std(uint32_t n, size_t sz) { do_heapify(n, sz, true, true); }
void heapify_unsorted_std(uint32_t n, size_t sz) { do_heapify(n, sz, false, true); }
BENCHMARK_PARAM(heapify_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_sorted_std, 1000)
BENCHMARK_PARAM(heapify_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_sorted_std, 100000)
BENCHMARK_PARAM(heapify_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_unsorted_std, 1000)
BENCHMARK_PARAM(heapify_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_unsorted_std, 100000)

int main(int argc, char** argv) {
  runBenchmarks();
  return 0;
}

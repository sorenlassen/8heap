/*
   brew install folly
   gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c
   g++ -g -std=c++14 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o h8Benchmark.cpp
*/

#include <cstddef>
#include <cstdint>
#include "H8.hpp"
#include "StdMinHeap.hpp"
#include <folly/Benchmark.h>

using namespace folly;

void fill(elem_type* ptr, size_t sz, bool sorted) {
  double f = 65536.0 / sz;
  for (size_t i = 0; i < sz; ++i) {
    ptr[i] = (elem_type)((sorted ? i : (sz - 1 - i)) * f);
  }
}

template<class Heap>
void heapify(uint32_t n, size_t sz, bool sorted) {
  typename Heap::elem_type x = 0;
  Heap h;
  typename Heap::elem_type* ptr = h.extend(sz);
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(ptr, sz, sorted);
    }
    h.heapify();
    x ^= h.top();
  }
  doNotOptimizeAway(x);
}

void heapify_h8_sorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, true); }
void heapify_h8_unsorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, false); }
void heapify_std_sorted(uint32_t n, size_t sz) { heapify<StdMinHeap>(n, sz, true); }
void heapify_std_unsorted(uint32_t n, size_t sz) { heapify<StdMinHeap>(n, sz, false); }
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

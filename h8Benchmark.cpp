/*
   brew install folly
   gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o h8Benchmark.cpp
*/

#include <cstddef>
#include <cstdint>
#include <limits>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "H8.hpp"
#include "StdMinHeap.hpp"
#include <folly/Benchmark.h>

using boost::iterators::counting_iterator;
using boost::iterators::reverse_iterator;
using boost::iterators::transform_iterator;
using namespace folly;

template<class CountType, class FunctionType>
auto iter(CountType n, FunctionType f) -> decltype(auto) {
  return transform_iterator<FunctionType, counting_iterator<CountType>>(n, f);
}

template<class Heap>
void fill(Heap& heap, typename Heap::size_type sz, bool ascending) {
  typedef typename Heap::size_type size_type;
  typedef typename Heap::elem_type elem_type;
  elem_type max = std::numeric_limits<elem_type>::max();
  double mult = (max + 1.0) / sz;
  auto f = [=](size_type i) { return boost::numeric_cast<elem_type>(i * mult); };
  auto begin = iter(size_type(0), f);
  auto end = begin + sz;
  if (ascending) {
    heap.append(begin, end);
  } else {
    heap.append(reverse_iterator(end), reverse_iterator(begin));
  }
}

template<class Heap>
void heapify(uint32_t n, size_t sz, bool ascending) {
  typedef typename Heap::elem_type elem_type;
  Heap h;
  elem_type x = 0;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(h, sz, ascending);
    }
    h.heapify();
    x ^= h.top();
    h.clear();
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
BENCHMARK_PARAM(heapify_h8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 10000000)
BENCHMARK_PARAM(heapify_h8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 1000)
BENCHMARK_PARAM(heapify_h8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 100000)
BENCHMARK_PARAM(heapify_h8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 10000000)

int main(int argc, char** argv) {
  runBenchmarks();
  return 0;
}

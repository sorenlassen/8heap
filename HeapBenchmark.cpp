/*
   brew install folly
   gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c &&
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o HeapBenchmark.cpp
*/

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "H8.hpp"
#include "Heap8.hpp"
#include "StdMinHeap.hpp"
#include <folly/Benchmark.h>

using boost::iterators::counting_iterator;
using boost::iterators::transform_iterator;
using namespace folly;

namespace {

template<class CountType, class FunctionType>
auto iter(CountType n, FunctionType f) -> decltype(auto) {
  return transform_iterator<FunctionType, counting_iterator<CountType>>(n, f);
}

template<class Appendable>
void fill(Appendable& out, typename Appendable::size_type sz, bool ascending) {
  typedef typename Appendable::size_type size_type;
  typedef typename Appendable::elem_type elem_type;
  elem_type max = std::numeric_limits<elem_type>::max();
  double mult = (max + 1.0) / sz;
  auto f = [=](size_type i) { return boost::numeric_cast<elem_type>(i * mult); };
  auto begin = iter(size_type(0), f);
  auto end = begin + sz;
  out.clear();
  if (ascending) {
    out.append(begin, end);
  } else {
    out.append(std::reverse_iterator(end), std::reverse_iterator(begin));
  }
}

// vector with added append() method
struct AppendableVector : public std::vector<uint16_t> {
  typedef value_type elem_type;
  template<class InputIterator>
  void append(InputIterator from, InputIterator to) {
    insert(end(), from, to);
  }
};

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
  }
  doNotOptimizeAway(x);
}

template<class Heap>
void heapsort(uint32_t n, size_t sz, bool ascending) {
  typedef typename Heap::elem_type elem_type;
  Heap h;
  std::vector<elem_type> result(sz);
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(h, sz, ascending);
    }
    h.heapify();
    for (size_t j = 0; j < sz; ++j) result[j] = h.pop();
  }
  doNotOptimizeAway(result[n % sz]);
}

void sort(uint32_t n, size_t sz, bool ascending) {
  typedef typename AppendableVector::elem_type elem_type;
  AppendableVector result;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(result, sz, ascending);
    }
    std::sort(result.begin(), result.end());
  }
  doNotOptimizeAway(result[n % sz]);
}

void heapify_h8_sorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, true); }
void heapify_h8_unsorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, false); }
void heapify_heap8_sorted(uint32_t n, size_t sz) { heapify<h8::Heap8>(n, sz, true); }
void heapify_heap8_unsorted(uint32_t n, size_t sz) { heapify<h8::Heap8>(n, sz, false); }
void heapify_std_sorted(uint32_t n, size_t sz) { heapify<StdMinHeap>(n, sz, true); }
void heapify_std_unsorted(uint32_t n, size_t sz) { heapify<StdMinHeap>(n, sz, false); }

void heapsort_h8_sorted(uint32_t n, size_t sz) { heapsort<H8>(n, sz, true); }
void heapsort_h8_unsorted(uint32_t n, size_t sz) { heapsort<H8>(n, sz, false); }
void heapsort_heap8_sorted(uint32_t n, size_t sz) { heapsort<h8::Heap8>(n, sz, true); }
void heapsort_heap8_unsorted(uint32_t n, size_t sz) { heapsort<h8::Heap8>(n, sz, false); }
void heapsort_std_sorted(uint32_t n, size_t sz) { heapsort<StdMinHeap>(n, sz, true); }
void heapsort_std_unsorted(uint32_t n, size_t sz) { heapsort<StdMinHeap>(n, sz, false); }

void sort_sorted(uint32_t n, size_t sz) { sort(n, sz, true); }
void sort_unsorted(uint32_t n, size_t sz) { sort(n, sz, false); }

} // namespace

BENCHMARK_PARAM(heapify_h8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 1000)
BENCHMARK_PARAM(heapify_h8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 100000)
BENCHMARK_PARAM(heapify_h8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_std_sorted, 10000000)
BENCHMARK_PARAM(heapify_h8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 1000)
BENCHMARK_PARAM(heapify_h8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 100000)
BENCHMARK_PARAM(heapify_h8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_heap8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapify_std_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(heapsort_h8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(sort_sorted, 1000)
BENCHMARK_PARAM(heapsort_h8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(sort_sorted, 100000)
BENCHMARK_PARAM(heapsort_h8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(sort_sorted, 10000000)
BENCHMARK_PARAM(heapsort_h8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(sort_unsorted, 1000)
BENCHMARK_PARAM(heapsort_h8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(sort_unsorted, 100000)
BENCHMARK_PARAM(heapsort_h8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapsort_heap8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(heapsort_std_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(sort_unsorted, 10000000)

int main(int argc, char** argv) {
  runBenchmarks();
  return 0;
}

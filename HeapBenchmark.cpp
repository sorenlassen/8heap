/*
   brew install folly gflags
   gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c &&
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags h8.o HeapBenchmark.cpp -o HeapBenchmark.out

   ./HeapBenchmark.out                                         # all
   ./HeapBenchmark.out --bm_regex push                         # only push
   ./HeapBenchmark.out --bm_regex std | awk '{print$1,$3,$4}'  # std w/o relative column
*/

#include "H8.hpp"
#include "Heap8.hpp"
#include "StdMinHeap.hpp"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <folly/Benchmark.h>
#include <gflags/gflags.h>

using namespace folly;

namespace {

std::default_random_engine gen(0);

template<class value_type>
struct Random {
  static std::uniform_int_distribution<value_type> distr;
};

template<class value_type>
std::uniform_int_distribution<value_type> Random<value_type>::distr(
  std::numeric_limits<value_type>::min(),
  std::numeric_limits<value_type>::max());

template<class value_type, class size_type>
std::function<value_type(size_type)>
transform_ascending(size_type sz) {
  const double mult = (std::numeric_limits<value_type>::max() + 1.0) / sz;
  return [mult](size_type i) { return boost::numeric_cast<value_type>(i * mult); };
}

template<class value_type, class size_type>
std::function<value_type(size_type)>
transform_random(size_type sz) {
  return [](size_type i) { return Random<value_type>::distr(gen); };
}

template<class CountType, class FunctionType>
auto iter(CountType n, FunctionType f) -> decltype(auto) {
  using boost::iterators::counting_iterator;
  using boost::iterators::transform_iterator;
  return transform_iterator<FunctionType, counting_iterator<CountType>>(n, f);
}

template<class Appendable>
void push(Appendable& out, typename Appendable::size_type sz, bool ascending) {
  typedef typename Appendable::size_type size_type;
  typedef typename Appendable::value_type value_type;
  auto transform = ascending
    ? transform_ascending<value_type, size_type>(sz)
    : transform_random<value_type, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  while (begin != end) out.push(*begin++);
}

template<class Appendable>
void fill(Appendable& out, typename Appendable::size_type sz, bool ascending) {
  typedef typename Appendable::size_type size_type;
  typedef typename Appendable::value_type value_type;
  auto transform = ascending
    ? transform_ascending<value_type, size_type>(sz)
    : transform_random<value_type, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  out.append(begin, end);
}

template<class Heap>
void push(uint32_t n, size_t sz, bool ascending) {
  typedef typename Heap::value_type value_type;
  Heap h;
  for (int i = 0; i < n; ++i) {
    push(h, sz, ascending);
    doNotOptimizeAway(h.top());
  }
}

template<class Heap>
void heapify(uint32_t n, size_t sz, bool ascending) {
  Heap h;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(h, sz, ascending);
    }
    h.heapify();
    doNotOptimizeAway(h.top());
  }
}

template<class Heap>
void heapsort(uint32_t n, size_t sz, bool ascending) {
  Heap h;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(h, sz, ascending);
    }
    h.heapify();
    h.sort();
    doNotOptimizeAway(h[0]);
  }
}

typedef uint16_t ValueType;

// vector with added append() method
struct AppendableVector : public std::vector<ValueType> {
  template<class InputIterator>
  void append(InputIterator from, InputIterator to) {
    insert(end(), from, to);
  }
};

void sort(uint32_t n, size_t sz, bool ascending) {
  AppendableVector result;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(result, sz, ascending);
    }
    std::sort(result.begin(), result.end());
    doNotOptimizeAway(result[0]);
  }
}

void push_h8_sorted(uint32_t n, size_t sz) { push<H8>(n, sz, true); }
void push_h8_unsorted(uint32_t n, size_t sz) { push<H8>(n, sz, false); }
void push_heap8_sorted(uint32_t n, size_t sz) { push<Heap8>(n, sz, true); }
void push_heap8_unsorted(uint32_t n, size_t sz) { push<Heap8>(n, sz, false); }
void push_std_sorted(uint32_t n, size_t sz) { push<StdMinHeap<>>(n, sz, true); }
void push_std_unsorted(uint32_t n, size_t sz) { push<StdMinHeap<>>(n, sz, false); }

void heapify_h8_sorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, true); }
void heapify_h8_unsorted(uint32_t n, size_t sz) { heapify<H8>(n, sz, false); }
void heapify_heap8_sorted(uint32_t n, size_t sz) { heapify<Heap8>(n, sz, true); }
void heapify_heap8_unsorted(uint32_t n, size_t sz) { heapify<Heap8>(n, sz, false); }
void heapify_std_sorted(uint32_t n, size_t sz) { heapify<StdMinHeap<>>(n, sz, true); }
void heapify_std_unsorted(uint32_t n, size_t sz) { heapify<StdMinHeap<>>(n, sz, false); }

void heapsort_h8_sorted(uint32_t n, size_t sz) { heapsort<H8>(n, sz, true); }
void heapsort_h8_unsorted(uint32_t n, size_t sz) { heapsort<H8>(n, sz, false); }
void heapsort_heap8_sorted(uint32_t n, size_t sz) { heapsort<Heap8>(n, sz, true); }
void heapsort_heap8_unsorted(uint32_t n, size_t sz) { heapsort<Heap8>(n, sz, false); }
void heapsort_std_sorted(uint32_t n, size_t sz) { heapsort<StdMinHeap<>>(n, sz, true); }
void heapsort_std_unsorted(uint32_t n, size_t sz) { heapsort<StdMinHeap<>>(n, sz, false); }

void sort_sorted(uint32_t n, size_t sz) { sort(n, sz, true); }
void sort_unsorted(uint32_t n, size_t sz) { sort(n, sz, false); }

} // namespace

BENCHMARK_PARAM(push_h8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(push_heap8_sorted, 1000)
BENCHMARK_RELATIVE_PARAM(push_std_sorted, 1000)
BENCHMARK_PARAM(push_h8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(push_heap8_sorted, 100000)
BENCHMARK_RELATIVE_PARAM(push_std_sorted, 100000)
BENCHMARK_PARAM(push_h8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(push_heap8_sorted, 10000000)
BENCHMARK_RELATIVE_PARAM(push_std_sorted, 10000000)
BENCHMARK_PARAM(push_h8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(push_heap8_unsorted, 1000)
BENCHMARK_RELATIVE_PARAM(push_std_unsorted, 1000)
BENCHMARK_PARAM(push_h8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(push_heap8_unsorted, 100000)
BENCHMARK_RELATIVE_PARAM(push_std_unsorted, 100000)
BENCHMARK_PARAM(push_h8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(push_heap8_unsorted, 10000000)
BENCHMARK_RELATIVE_PARAM(push_std_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
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
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  runBenchmarks();
  return 0;
}

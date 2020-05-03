#include "Heap8Aux.hpp"
#include <utility>
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

typedef Heap8Aux<uint32_t> Aux;

namespace {

std::default_random_engine gen;

template<class value_type>
struct Random {
  static std::uniform_int_distribution<value_type> distr;
};


template<class value_type>
std::uniform_int_distribution<value_type> Random<value_type>::distr(
  std::numeric_limits<value_type>::min(),
  std::numeric_limits<value_type>::max());

template<class value_type, class aux_type, class size_type>
std::function<std::pair<value_type, aux_type>(size_type)>
transform_ascending_pair(size_type sz) {
  const double mult1 = (std::numeric_limits<value_type>::max() + 1.0) / sz;
  const double mult2 = (std::numeric_limits<aux_type>::max() + 1.0) / sz;
  return [mult1, mult2](size_type i) { 
    return std::make_pair(boost::numeric_cast<value_type>(i * mult1), 
                          boost::numeric_cast<aux_type>(i * mult2));
  };
}

template<class value_type, class aux_type, class size_type>
std::function<std::pair<value_type, aux_type>(size_type)>
transform_random_pair(size_type sz) {
  return [](size_type i) { 
    return std::make_pair(Random<value_type>::distr(gen),
                          Random<aux_type>::distr(gen)); 
  };
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
    ? transform_ascending_pair<value_type, uint32_t, size_type>(sz)
    : transform_random_pair<value_type, uint32_t, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  while (begin != end) out.push_entry(*begin++);
}

template<class Appendable>
void fill(Appendable& out, typename Appendable::size_type sz, bool ascending) {
  typedef typename Appendable::size_type size_type;
  typedef typename Appendable::value_type value_type;
  typedef typename Appendable::shadow_type aux_type;
  auto transform = ascending
    ? transform_ascending_pair<value_type, aux_type, size_type>(sz)
    : transform_random_pair<value_type, aux_type, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  out.append_entries(begin, end);
}

template<class Heap>
void push(uint32_t n, size_t sz, bool ascending) {
  typedef typename Heap::value_type value_type;
  Heap h;
  value_type x = 0;
  for (int i = 0; i < n; ++i) {
    push(h, sz, ascending);
    x ^= h.top_entry().first;
  }
  doNotOptimizeAway(x);
}

template<class Heap>
void heapify(uint32_t n, size_t sz, bool ascending) {
  typedef typename Heap::value_type value_type;
  Heap h;
  value_type x = 0;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(h, sz, ascending);
    }
    h.heapify();
    x ^= h.top_entry().first;
  }
  doNotOptimizeAway(x);
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
  }
  doNotOptimizeAway(h[0]);
}

// vector with added append() method
struct AppendableVector : public std::vector<std::pair<uint16_t, uint32_t>> {
  typedef uint16_t value_type;
  typedef uint32_t shadow_type;
  template<class InputIterator>
  void append_entries(InputIterator from, InputIterator to) {
    insert(end(), from, to);
  }
};

bool sortPair(const std::pair<uint16_t, uint32_t> &v1, 
	      const std::pair<uint16_t, uint32_t> &v2) {
  return v1.first < v2.first;
}

void sort(uint32_t n, size_t sz, bool ascending) {
  AppendableVector result;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(result, sz, ascending);
    }
    std::sort(result.begin(), result.end(), sortPair);
  }
  doNotOptimizeAway(result[0]);
}

void push_heap8aux_sorted(uint32_t n, size_t sz) { push<Aux>(n, sz, true); }
void push_heap8aux_unsorted(uint32_t n, size_t sz) { push<Aux>(n, sz, true); }


void heapify_heap8aux_sorted(uint32_t n, size_t sz) { heapify<Aux>(n, sz, true); }
void heapify_heap8aux_unsorted(uint32_t n, size_t sz) { heapify<Aux>(n, sz, true); }

void heapsort_heap8aux_sorted(uint32_t n, size_t sz) { heapsort<Aux>(n, sz, true); }
void heapsort_heap8aux_unsorted(uint32_t n, size_t sz) { heapsort<Aux>(n, sz, true); }


} // namespace

BENCHMARK_PARAM(push_heap8aux_sorted, 1000)
BENCHMARK_PARAM(push_heap8aux_sorted, 100000)
BENCHMARK_PARAM(push_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(heapify_heap8aux_sorted, 1000)
BENCHMARK_PARAM(heapify_heap8aux_sorted, 100000)
BENCHMARK_PARAM(heapify_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 1000)
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 100000)
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 10000000)

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  runBenchmarks();
  return 0;
}

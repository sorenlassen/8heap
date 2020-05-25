#include "Heap8Aux.hpp"
#include "Heap8Embed.hpp"
#include "StdMinHeapMap.hpp"
#include "U48.hpp"
#include "FirstCompare.hpp"
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <utility>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <folly/Benchmark.h>
#include <gflags/gflags.h>

using namespace folly;

namespace {

std::default_random_engine gen;

template<class key_type>
struct Random {
  static std::uniform_int_distribution<key_type> distr;
};

template<class key_type>
std::uniform_int_distribution<key_type> Random<key_type>::distr(
  std::numeric_limits<key_type>::min(),
  std::numeric_limits<key_type>::max());

template<class key_type, class mapped_type, class size_type>
std::function<std::pair<key_type, mapped_type>(size_type)>
transform_ascending_pair(size_type sz) {
  const double mult = (std::numeric_limits<key_type>::max() + 1.0) / sz;
  return [mult](size_type i) {
    key_type key = boost::numeric_cast<key_type>(i * mult);
    return std::make_pair(key, static_cast<mapped_type>(key));
  };
}

template<class key_type, class mapped_type, class size_type>
std::function<std::pair<key_type, mapped_type>(size_type)>
transform_random_pair(size_type sz) {
  return [](size_type i) {
    key_type key = Random<key_type>::distr(gen);
    return std::make_pair(key, static_cast<mapped_type>(key));
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
  typedef typename Appendable::key_type key_type;
  typedef typename Appendable::mapped_type mapped_type;
  auto transform = ascending
    ? transform_ascending_pair<key_type, mapped_type, size_type>(sz)
    : transform_random_pair<key_type, mapped_type, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  while (begin != end) out.push_entry(*begin++);
}

template<class Appendable>
void fill(Appendable& out, typename Appendable::size_type sz, bool ascending) {
  typedef typename Appendable::size_type size_type;
  typedef typename Appendable::key_type key_type;
  typedef typename Appendable::mapped_type mapped_type;
  auto transform = ascending
    ? transform_ascending_pair<key_type, mapped_type, size_type>(sz)
    : transform_random_pair<key_type, mapped_type, size_type>(sz);
  auto begin = iter(size_type(0), transform);
  auto end = begin + sz;
  out.clear();
  out.append_entries(begin, end);
}

template<typename Pair>
void doNotOptimizeAway_pair(Pair const& pair) {
  doNotOptimizeAway(pair.first + pair.second);
}

template<class Heap>
void push(uint32_t n, size_t sz, bool ascending) {
  Heap h;
  for (int i = 0; i < n; ++i) {
    push(h, sz, ascending);
    doNotOptimizeAway_pair(h.top_entry());
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
    doNotOptimizeAway_pair(h.top_entry());
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
    doNotOptimizeAway_pair(h.entry(0));
  }
}

typedef uint16_t KeyType;
typedef U48 MappedType;

// vector with added append() method
struct AppendableVector : public std::vector<std::pair<KeyType, MappedType>> {
  typedef KeyType key_type;
  typedef MappedType mapped_type;
  template<class InputIterator>
  void append_entries(InputIterator from, InputIterator to) {
    insert(end(), from, to);
  }
};

void sort(uint32_t n, size_t sz, bool ascending) {
  typedef typename AppendableVector::key_type key_type;
  typedef typename AppendableVector::mapped_type mapped_type;
  FirstCompare<key_type, mapped_type> cmp;
  AppendableVector result;
  for (int i = 0; i < n; ++i) {
    BENCHMARK_SUSPEND {
      fill(result, sz, ascending);
    }
    std::sort(result.begin(), result.end(), cmp);
    doNotOptimizeAway_pair(result[0]);
  }
}

typedef Heap8Aux<MappedType> Aux;
typedef Heap8Embed<MappedType> Embed;
typedef StdMinHeapMap<MappedType> Std;

void push_heap8aux_sorted(uint32_t n, size_t sz) { push<Aux>(n, sz, true); }
void push_heap8embed_sorted(uint32_t n, size_t sz) { push<Embed>(n, sz, true); }
void push_stdheapmap_sorted(uint32_t n, size_t sz) { push<Std>(n, sz, true); }
void push_heap8aux_unsorted(uint32_t n, size_t sz) { push<Aux>(n, sz, true); }
void push_heap8embed_unsorted(uint32_t n, size_t sz) { push<Embed>(n, sz, true); }
void push_stdheapmap_unsorted(uint32_t n, size_t sz) { push<Std>(n, sz, true); }
void heapify_heap8aux_sorted(uint32_t n, size_t sz) { heapify<Aux>(n, sz, true); }
void heapify_heap8embed_sorted(uint32_t n, size_t sz) { heapify<Embed>(n, sz, true); }
void heapify_stdheapmap_sorted(uint32_t n, size_t sz) { heapify<Std>(n, sz, true); }
void heapify_heap8aux_unsorted(uint32_t n, size_t sz) { heapify<Aux>(n, sz, true); }
void heapify_heap8embed_unsorted(uint32_t n, size_t sz) { heapify<Embed>(n, sz, true); }
void heapify_stdheapmap_unsorted(uint32_t n, size_t sz) { heapify<Std>(n, sz, true); }
void heapsort_heap8aux_sorted(uint32_t n, size_t sz) { heapsort<Aux>(n, sz, true); }
void heapsort_heap8embed_sorted(uint32_t n, size_t sz) { heapsort<Embed>(n, sz, true); }
void heapsort_stdheapmap_sorted(uint32_t n, size_t sz) { heapsort<Std>(n, sz, true); }
void heapsort_heap8aux_unsorted(uint32_t n, size_t sz) { heapsort<Aux>(n, sz, true); }
void heapsort_heap8embed_unsorted(uint32_t n, size_t sz) { heapsort<Embed>(n, sz, true); }
void heapsort_stdheapmap_unsorted(uint32_t n, size_t sz) { heapsort<Std>(n, sz, true); }

} // namespace

BENCHMARK_PARAM(push_heap8aux_sorted, 1000)
BENCHMARK_PARAM(push_heap8embed_sorted, 1000)
BENCHMARK_PARAM(push_stdheapmap_sorted, 1000)
BENCHMARK_PARAM(push_heap8aux_sorted, 100000)
BENCHMARK_PARAM(push_heap8embed_sorted, 100000)
BENCHMARK_PARAM(push_stdheapmap_sorted, 100000)
BENCHMARK_PARAM(push_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(push_heap8embed_sorted, 10000000)
BENCHMARK_PARAM(push_stdheapmap_sorted, 10000000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(push_heap8embed_unsorted, 1000)
BENCHMARK_PARAM(push_stdheapmap_unsorted, 1000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(push_heap8embed_unsorted, 100000)
BENCHMARK_PARAM(push_stdheapmap_unsorted, 100000)
BENCHMARK_PARAM(push_heap8aux_unsorted, 10000000)
BENCHMARK_PARAM(push_heap8embed_unsorted, 10000000)
BENCHMARK_PARAM(push_stdheapmap_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(heapify_heap8aux_sorted, 1000)
BENCHMARK_PARAM(heapify_heap8embed_sorted, 1000)
BENCHMARK_PARAM(heapify_stdheapmap_sorted, 1000)
BENCHMARK_PARAM(heapify_heap8aux_sorted, 100000)
BENCHMARK_PARAM(heapify_heap8embed_sorted, 100000)
BENCHMARK_PARAM(heapify_stdheapmap_sorted, 100000)
BENCHMARK_PARAM(heapify_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(heapify_heap8embed_sorted, 10000000)
BENCHMARK_PARAM(heapify_stdheapmap_sorted, 10000000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(heapify_heap8embed_unsorted, 1000)
BENCHMARK_PARAM(heapify_stdheapmap_unsorted, 1000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(heapify_heap8embed_unsorted, 100000)
BENCHMARK_PARAM(heapify_stdheapmap_unsorted, 100000)
BENCHMARK_PARAM(heapify_heap8aux_unsorted, 10000000)
BENCHMARK_PARAM(heapify_heap8embed_unsorted, 10000000)
BENCHMARK_PARAM(heapify_stdheapmap_unsorted, 10000000)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 1000)
BENCHMARK_PARAM(heapsort_heap8embed_sorted, 1000)
BENCHMARK_PARAM(heapsort_stdheapmap_sorted, 1000)
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 100000)
BENCHMARK_PARAM(heapsort_heap8embed_sorted, 100000)
BENCHMARK_PARAM(heapsort_stdheapmap_sorted, 100000)
BENCHMARK_PARAM(heapsort_heap8aux_sorted, 10000000)
BENCHMARK_PARAM(heapsort_heap8embed_sorted, 10000000)
BENCHMARK_PARAM(heapsort_stdheapmap_sorted, 10000000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 1000)
BENCHMARK_PARAM(heapsort_heap8embed_unsorted, 1000)
BENCHMARK_PARAM(heapsort_stdheapmap_unsorted, 1000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 100000)
BENCHMARK_PARAM(heapsort_heap8embed_unsorted, 100000)
BENCHMARK_PARAM(heapsort_stdheapmap_unsorted, 100000)
BENCHMARK_PARAM(heapsort_heap8aux_unsorted, 10000000)
BENCHMARK_PARAM(heapsort_heap8embed_unsorted, 10000000)
BENCHMARK_PARAM(heapsort_stdheapmap_unsorted, 10000000)

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  runBenchmarks();
  return 0;
}

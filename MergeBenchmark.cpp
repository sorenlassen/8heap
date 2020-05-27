/*
   brew install google-benchmark
   g++ -std=c++17 -msse4 -O2 -DNDEBUG -lbenchmark -lbenchmark_main MergeBenchmark.cpp
   ./a.out
*/

#include "Heap8Aux.hpp"
#include "Heap8Embed.hpp"
#include "StdMinHeapMap.hpp"
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <vector>
#include <benchmark/benchmark.h>

namespace {

typedef uint16_t KeyType;
typedef uint16_t MappedType;

struct Record {
  size_t size;
  KeyType const* list;
  size_t offset;
};

// Table must behave like vector<vector<KeyType>>.
// OutputIterator type must be pair<KeyType, MappedType>.
template<typename HeapMap, typename Table, typename OutputIterator>
void merge(HeapMap& heap_map, Table table, OutputIterator out) {
  assert(heap_map.size() == 0);

  std::vector<Record> records;
  records.reserve(table.size());
  for (auto const& x : table) {
    records.push_back({x.size(), x.data(), 0});
  }
  assert(table.size() == records.size());

  heap_map.extend(records.size());
  for (size_t i = 0; i < records.size(); ++i) {
    auto const& r = records[i];
    assert(r.size > 0); // for simplicity we assume every list is non-empty
    heap_map.set_entry(i, {r.list[0], i});
  }
  heap_map.heapify();

  while (heap_map.size() > 0) {
    auto index = heap_map.top_index();
    auto e = heap_map.entry(index);
    *out++ = e;
    auto &r = records[e.second];
    assert(r.offset < r.size);
    r.offset++;
    if (r.offset == r.size) {
      auto d = heap_map.pop_entry();
      assert(d == e);
    } else {
      // Should be faster than pop_entry followed by push_entry.
      heap_map.push_down(r.list[r.offset], e.second, index);
    }
  }
}

template<typename OutputIterator>
void initRandomArray(int seed, size_t sz, OutputIterator out) {
  std::default_random_engine gen(seed);
  std::uniform_int_distribution<uint16_t> distr(0, std::numeric_limits<uint16_t>::max());
  while (sz--) *out++ = distr(gen);
}

std::vector<KeyType> randomVector(int seed, size_t sz) {
  std::vector<KeyType> v;
  v.reserve(sz);
  initRandomArray(seed, sz, std::back_inserter(v));
  assert(v.size() == sz);
  return v;
}

std::vector<KeyType> randomSortedVector(int seed, size_t sz) {
  std::vector<KeyType> v = randomVector(seed, sz);
  std::sort(v.begin(), v.end());
  return v;
}

template<typename Pair>
void doNotOptimizeAway_pair(Pair const& pair) {
  benchmark::DoNotOptimize(pair.first + pair.second);
}

template<class HeapMap>
void bm_merge(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    auto count = state.range(0);
    auto size = state.range(1);
    std::vector<std::vector<KeyType>> table;
    table.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      table.push_back(randomSortedVector(i, size));
    }
    HeapMap heap_map;
    std::vector<typename HeapMap::entry_type> out;
    out.reserve(count * size);
    state.ResumeTiming();
    merge(heap_map, table, std::back_inserter(out));
    doNotOptimizeAway_pair(out[0]);
  }
}

typedef Heap8Aux<MappedType> Aux;
typedef Heap8Embed<MappedType> Embed;
typedef StdMinHeapMap<MappedType> Std;

// Parameters in ascending order.
const std::vector<int64_t> kCounts = { 10, 100 };
const std::vector<int64_t> kSizes = { 32000, 320000 };
void Arguments(benchmark::internal::Benchmark* b) {
  for (int64_t c : kCounts)
    for (int64_t s : kSizes)
      b->Args({c, s});
}

} // namespace

BENCHMARK_TEMPLATE(bm_merge, Aux)->Apply(Arguments);
BENCHMARK_TEMPLATE(bm_merge, Embed)->Apply(Arguments);
BENCHMARK_TEMPLATE(bm_merge, Std)->Apply(Arguments);

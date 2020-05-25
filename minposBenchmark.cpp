/*
   brew install google-benchmark
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lbenchmark minposBenchmark.cpp
*/

#include "minpos.h"
#include "align.h"
#include <cstdint>
#include <limits>
#include <random>
#include <vector>
#include <benchmark/benchmark.h>

namespace {

constexpr size_t kAlign = 64; // cache line

uint16_t* vs;

void initData(size_t sz) {
  vs = (uint16_t*)aligned_alloc(kAlign, sz * sizeof(uint16_t));
  std::default_random_engine gen;
  std::uniform_int_distribution<uint16_t> distr(0, std::numeric_limits<uint16_t>::max());
  for (int i = 0; i < sz; ++i) vs[i] = distr(gen);
}

constexpr size_t kLineLen = kAlign / sizeof(uint16_t);

template<class F>
void bm_minpos(benchmark::State& state, F fn) {
  for (auto _ : state) {
    minpos_type x = 0;
    for (int j = 0; j + kLineLen <= state.range(0); j += kLineLen) {
      x ^= fn(vs + j);
    }
    benchmark::DoNotOptimize(x);
  }
}

// Parameters in ascending order.
const std::vector<const size_t> kParams = { 32000, 3200000, 320000000 };
const size_t kLargestParam = kParams.back();

void Arguments(benchmark::internal::Benchmark* b) {
  for (size_t p : kParams) b->Arg(p);
}

} // namespace

BENCHMARK_CAPTURE(bm_minpos,  8, minpos8 )->Apply(Arguments);
BENCHMARK_CAPTURE(bm_minpos, 16, minpos16)->Apply(Arguments);
BENCHMARK_CAPTURE(bm_minpos, 32, minpos32)->Apply(Arguments);

int main(int argc, char** argv) {
  initData(kLargestParam);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}

/*
   brew install folly gflags
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -c Sort8.cpp &&
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags Sort8.o Sort8Benchmark.cpp
*/

#include "Sort8.hpp"
#include <emmintrin.h>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <random>
#include <folly/Benchmark.h>
#include <gflags/gflags.h>

using namespace folly;
using namespace std;

namespace {

constexpr size_t kArity = 8;
constexpr size_t kCount = 10000;
__m128i mms[kCount];

void initData() {
  default_random_engine gen;
  uniform_int_distribution<uint16_t> distr(0, numeric_limits<uint16_t>::max());
  uint16_t* vs = reinterpret_cast<uint16_t*>(mms);
  for (size_t i = 0; i < kCount * kArity; ++i) vs[i] = distr(gen);
}

} // namespace

BENCHMARK(sort8) {
  __m128i x = _mm_set1_epi16(0);
  for (size_t j = 0; j < kCount; ++j) {
    x ^= sort8(mms[j]);
  }
  doNotOptimizeAway(x);
}

BENCHMARK_RELATIVE(std_sort) {
  __m128i x = _mm_set1_epi16(0);
  for (size_t j = 0; j < kCount; ++j) {
    __m128i tmp = mms[j];
    uint16_t* vs = reinterpret_cast<uint16_t*>(&tmp);
    std::sort(vs, vs + kArity);
    x ^= sort8(tmp);
  }
  doNotOptimizeAway(x);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  initData();
  runBenchmarks();
  return 0;
}

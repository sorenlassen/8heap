/*
   brew install folly gflags
   g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags minposBenchmark.cpp
*/

#include "minpos.h"
#include <stdalign.h> // no <cstdalign> on mac
#include <cstdint>
#include <limits>
#include <random>
#include <folly/Benchmark.h>
#include <gflags/gflags.h>

using namespace folly;

namespace {

#if defined(__clang__) && __clang_major__ < 11
// C11 aligned_alloc is not available on mac.
void* aligned_alloc(size_t alignment, size_t sz) {
  void* ptr = NULL;
  return 0 == posix_memalign(&ptr, alignment, sz) ? ptr : NULL;
}
#endif

constexpr std::size_t kAlign = 64; // cache line

uint16_t* vs;

void initData(std::size_t sz) {
  vs = (uint16_t*)aligned_alloc(kAlign, sz * sizeof(uint16_t));
  std::default_random_engine gen;
  std::uniform_int_distribution<uint16_t> distr(0, std::numeric_limits<uint16_t>::max());
  for (int i = 0; i < sz; ++i) vs[i] = distr(gen);
}

constexpr std::size_t kLineLen = kAlign / sizeof(uint16_t);

void bm_minpos8(uint32_t n, std::size_t sz) {
  minpos_type x = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j + kLineLen <= sz; j += kLineLen) {
      x ^= minpos8(vs + j);
    }
  }
  doNotOptimizeAway(x);
}

void bm_minpos16(uint32_t n, std::size_t sz) {
  minpos_type x = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j + kLineLen <= sz; j += kLineLen) {
      x ^= minpos16(vs + j);
    }
  }
  doNotOptimizeAway(x);
}

void bm_minpos32(uint32_t n, std::size_t sz) {
  minpos_type x = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j + kLineLen <= sz; j += kLineLen) {
      x ^= minpos32(vs + j);
    }
  }
  doNotOptimizeAway(x);
}

} // namespace

BENCHMARK_PARAM         (bm_minpos8,  32000);
BENCHMARK_RELATIVE_PARAM(bm_minpos16, 32000);
BENCHMARK_RELATIVE_PARAM(bm_minpos32, 32000);
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM         (bm_minpos8 , 3200000);
BENCHMARK_RELATIVE_PARAM(bm_minpos16, 3200000);
BENCHMARK_RELATIVE_PARAM(bm_minpos32, 3200000);
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM         (bm_minpos8 , 320000000);
BENCHMARK_RELATIVE_PARAM(bm_minpos16, 320000000);
BENCHMARK_RELATIVE_PARAM(bm_minpos32, 320000000);
constexpr std::size_t kLargestParam = 320000000;

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  initData(kLargestParam);
  runBenchmarks();
  return 0;
}

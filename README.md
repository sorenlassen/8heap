# 8heap
[![Build Status](https://api.travis-ci.org/sorenlassen/8heap.svg?branch=master)](https://travis-ci.org/sorenlassen/8heap)

An implementation of an 8-ary heap using SSE instruction phminposuw to calculate the horizontal minimum of unsigned 16 bit integers. The implementation provides faster delete minimum and heapify operations over a standard binary heap by leveraging the 8-ary trees shallower depth without paying the cost of the minimum of 8 elements due to the speed up from the SSE4 instruction.

# Requirements

- [CMake](https://cmake.org/download/) >= 3.14
- [Boost](https://www.boost.org/) >= 1.65

You can install these with
```shell
brew install cmake boost
```

# Installation

Installing from this git repository is as simple as
```shell
git clone https://github.com/sorenlassen/8heap.git
```

# Build, Tests, Benchmarks

You can either use make or CMake to run.

## Make

To use Make you first need to install googletest
```shell
pushd $HOME # or whereever you want to clone googletest
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build/
cmake ..
make
make install
popd
```

Then run the following command
```shell
make runtests
```

To run the benchmarks install
```shell
brew install google-benchmark gflags folly
```
and then
```shell
make runbenchmarks
```

## CMake

To use CMake you first need to install gflags
```shell
brew install gflags
```

Then run the following commands
```shell
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make runtests
```

To benchmark, still in the build directory, clean the test libraries built above
```shell
make clean
```
and then
```shell
cmake -DCMAKE_BUILD_TYPE=Release ..
make runbenchmarks
make runfollybenchmarks
```

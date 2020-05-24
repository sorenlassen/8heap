# 8heap
[![Build Status](https://api.travis-ci.org/sorenlassen/8heap.svg?branch=master)](https://travis-ci.org/sorenlassen/8heap)

An implementation of an 8-ary heap using SSE instruction phminposuw to calculate the horizontal minimum of unsigned 16 bit integers. The implementation provides faster delete minimum and heapify operations over a standard binary heap by leveraging the 8-ary trees shallower depth without paying the cost of the minimum of 8 elements due to the speed up from the SSE4 instruction.

# Requirements

- [CMake](https://cmake.org/download/) >= 3.11
- [Boost](https://www.boost.org/) >= 1.65

# Installation

Installing from this git repository is as simple as
```bash
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

To use CMake you first need to install gflags and openssl
```shell
brew install openssl gflags
```
and then capture the libssl library path
```shell
LIBSSL_PATH=$(brew list openssl | grep /lib/libssl.a)
```
which is used in the call to cmake below.
(The gflags and openssl stuff is needed to configure the folly library and
folly benchmarks. Given these complications we may remove the folly dependency
and only use Google's benchmark library instead.)

Then run the following commands
```shell
mkdir build && cd build
cmake -DOPENSSL_ROOT_DIR=${LIBSSL_PATH%/lib/libssl.a} -DCMAKE_BUILD_TYPE=Debug ..
make runtests
```

To benchmark, still in the build directory, clean the test libraries built above
```shell
make clean
```
and then
```shell
cmake -DOPENSSL_ROOT_DIR=${LIBSSL_PATH%/lib/libssl.a} -DCMAKE_BUILD_TYPE=Release ..
make runbenchmarks
```

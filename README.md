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

# Building and Tests

You can either use make or CMake to run.

## Make

With Make simply run the following command
```bash
make runtests
```

## CMake
With CMake simply run the following commands
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make runtests
```

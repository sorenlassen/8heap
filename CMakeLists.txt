cmake_minimum_required(VERSION 3.11)

project(8HEAP)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions("-DHAS_BOOST")
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.5.0
)
FetchContent_GetProperties(benchmark)
if(NOT benchmark_POPULATED)
  FetchContent_Populate(benchmark)
  add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif()

find_package(gflags REQUIRED)

if(APPLE)
  find_package(folly REQUIRED)
  set(FOLLYBENCHMARK Folly::follybenchmark)
else()
  FetchContent_Declare(
    folly
    GIT_REPOSITORY https://github.com/facebook/folly.git
    GIT_TAG        v2020.05.18.00
  )
  FetchContent_GetProperties(folly)
  if(NOT folly_POPULATED)
    FetchContent_Populate(folly)
    add_subdirectory(${folly_SOURCE_DIR} ${folly_BINARY_DIR})
  endif()
  set(FOLLYBENCHMARK follybenchmark)
endif()

# Set compiler flags
add_compile_options(-msse4)

# Add object file libraries
add_library(h8 h8.c)
add_library(Sort8 Sort8.cpp)

# Tests
add_executable(minposTest minposTest.cpp)
target_link_libraries(minposTest LINK_PUBLIC gtest_main gtest)

add_executable(U48Test U48Test.cpp)
target_link_libraries(U48Test LINK_PUBLIC gtest_main gtest)

add_executable(h8Test h8Test.cpp)
target_link_libraries(h8Test LINK_PUBLIC gtest_main gtest h8)

add_executable(HeapTest HeapTest.cpp)
target_link_libraries(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest h8)

add_executable(HeapMapTest HeapMapTest.cpp)
target_link_libraries(HeapMapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

add_executable(Sort8Test Sort8Test.cpp)
target_link_libraries(Sort8Test LINK_PUBLIC gtest_main gtest Sort8)

add_custom_target(runtests
  COMMAND minposTest
  COMMAND U48Test
  COMMAND h8Test
  COMMAND HeapTest
  COMMAND HeapMapTest
  COMMAND Sort8Test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run tests in ${CMAKE_CURRENT_SOURCE_DIR}"
)

# Benchmarks
add_executable(minposBenchmark minposBenchmark.cpp)
target_link_libraries(minposBenchmark benchmark)

add_executable(MergeBenchmark MergeBenchmark.cpp)
target_link_libraries(MergeBenchmark benchmark benchmark_main)

add_custom_target(runbenchmarks
  COMMAND minposBenchmark
  COMMAND MergeBenchmark
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run benchmarks in ${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(minposFollyBenchmark minposFollyBenchmark.cpp)
target_link_libraries(minposFollyBenchmark ${FOLLYBENCHMARK} gflags)

add_executable(HeapBenchmark HeapBenchmark.cpp)
target_link_libraries(HeapBenchmark ${FOLLYBENCHMARK} gflags h8)

add_executable(HeapMapBenchmark HeapMapBenchmark.cpp)
target_link_libraries(HeapMapBenchmark ${FOLLYBENCHMARK} gflags)

add_executable(Sort8Benchmark Sort8Benchmark.cpp)
target_link_libraries(Sort8Benchmark ${FOLLYBENCHMARK} gflags Sort8)

add_custom_target(runfollybenchmarks
  COMMAND minposFollyBenchmark
  COMMAND HeapBenchmark
  COMMAND HeapMapBenchmark
  COMMAND Sort8Benchmark
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run folly benchmarks in ${CMAKE_CURRENT_SOURCE_DIR}"
)

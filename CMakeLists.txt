cmake_minimum_required(VERSION 3.11)

project(8HEAP)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

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

# Set compiler flags
add_compile_options(-msse4)

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions("-DHAS_BOOST")
endif()

# Add object file libraries
add_library(h8 h8.c)
add_library(Sort8 Sort8.cpp)

# Tests
add_executable(HeapAuxTest HeapAuxTest.cpp)
target_link_libraries(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

add_executable(HeapTest HeapTest.cpp)
target_link_libraries(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest h8)

add_executable(h8Test h8Test.cpp)
target_link_libraries(h8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest h8)

add_executable(Sort8Test Sort8Test.cpp)
target_link_libraries(Sort8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest Sort8)

add_executable(minposTest minposTest.cpp)
target_link_libraries(minposTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

add_executable(U48Test U48Test.cpp)
target_link_libraries(U48Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

add_custom_target(runtests
  COMMAND HeapAuxTest
  COMMAND HeapTest
  COMMAND h8Test
  COMMAND Sort8Test
  COMMAND minposTest
  COMMAND U48Test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run tests in ${CMAKE_CURRENT_SOURCE_DIR}"
)

# Benchmarks
add_executable(minposBenchmark minposBenchmark.cpp)
target_link_libraries(minposBenchmark LINK_PUBLIC ${Boost_LIBRARIES} benchmark)

add_custom_target(runbenchmarks
  COMMAND minposBenchmark
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run benchmarks in ${CMAKE_CURRENT_SOURCE_DIR}"
)

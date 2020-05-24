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

# Add Executable
add_executable(HeapAuxTest HeapAuxTest.cpp)
add_executable(HeapTest HeapTest.cpp)
add_executable(h8Test h8Test.cpp)
add_executable(Sort8Test Sort8Test.cpp)
add_executable(minposTest minposTest.cpp)
add_executable(U48Test U48Test.cpp)

# Link libraries
target_link_libraries(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)
target_link_libraries(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest h8)
target_link_libraries(h8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest h8)
target_link_libraries(Sort8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest Sort8)
target_link_libraries(minposTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)
target_link_libraries(U48Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

# Add runtests command
add_custom_target(runtests
  COMMAND HeapAuxTest
  COMMAND HeapTest
  COMMAND h8Test
  COMMAND Sort8Test
  COMMAND minposTest
  COMMAND U48Test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run generated tests in ${CMAKE_CURRENT_SOURCE_DIR}"
)

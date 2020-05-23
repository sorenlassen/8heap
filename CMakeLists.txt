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
add_compile_options(-g -msse4)

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions("-DHAS_BOOST")
endif()

# Add object file libraries
add_library(H8 h8.c h8.h v128.h minpos.h align.h)
add_library(Sort8 Sort8.cpp Sort8.hpp minpos.h)

# Add Executable
add_executable(HeapAuxTest HeapAuxTest.cpp Heap8Aux.hpp
			         minpos.h v128.h align.h)
add_executable(HeapTest HeapTest.cpp StdMinHeap.hpp
			         Heap8.hpp Heap8Aux.hpp H8.hpp
			         minpos.h v128.h align.h h8.h)
add_executable(H8Test h8Test.cpp minpos.h h8.h)
add_executable(Sort8Test Sort8Test.cpp Sort8.hpp v128.h)
add_executable(minposTest minposTest.cpp v128.h minpos.h)

# Link libraries
TARGET_LINK_LIBRARIES(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)
TARGET_LINK_LIBRARIES(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest H8)
TARGET_LINK_LIBRARIES(H8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest H8)
TARGET_LINK_LIBRARIES(Sort8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest Sort8)
TARGET_LINK_LIBRARIES(minposTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest)

# Add runtests command
add_custom_target(runtests
	COMMAND HeapAuxTest
	COMMAND HeapTest
	COMMAND H8Test
	COMMAND Sort8Test
	COMMAND minposTest
  	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  	COMMENT "run generated tests in ${CMAKE_CURRENT_SOURCE_DIR}"
)

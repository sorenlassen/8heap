cmake_minimum_required(VERSION 3.9)

# set the project name
project(8HEAP)

# add the executable
set(CMAKE_CXX_COMPILER g++)

# Set compiler flags
set (CMAKE_CXX_FLAGS "-g -std=c++17")
set (CMAKE_C_FLAGS "-g -std=c11")

# Libraries to Include
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -msse4 -O2")

# Add boost
find_package(Boost REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# Add google test
find_package(GTest REQUIRED)

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
TARGET_LINK_LIBRARIES(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main) 
TARGET_LINK_LIBRARIES(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main H8) 
TARGET_LINK_LIBRARIES(H8Test LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main H8) 
TARGET_LINK_LIBRARIES(Sort8Test LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main Sort8) 
TARGET_LINK_LIBRARIES(minposTest LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main) 
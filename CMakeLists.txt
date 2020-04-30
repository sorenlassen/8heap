cmake_minimum_required(VERSION 3.9)

# set the project name
project(8HEAP)

# add the executable
set(CMAKE_CXX_COMPILER g++)

# Set compiler flags
set (CMAKE_CXX_FLAGS "-g -std=c++17")

# Libraries to Include
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -msse4")

# Add boost
find_package(Boost REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# Add google test
find_package(GTest REQUIRED)

# Add Executable
add_executable(HeapAuxTest HeapAuxTest.cpp Heap8Aux.hpp minpos.h v128.h align.h)

# Link libraries
TARGET_LINK_LIBRARIES(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main) 

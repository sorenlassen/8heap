cmake_minimum_required(VERSION 2.8.2)

project(DP_Example)

include(CTest)
set (CMAKE_CXX_STANDARD 17)

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

include(DownloadProject.cmake)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()

# Trivial example using gtest and gmock
set (CMAKE_CXX_FLAGS "-g -std=c++17")
set (CMAKE_C_FLAGS "-g -std=c11")
# Libraries to Include
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -msse4.1 -O2")

# Add boost
#find_package(Boost REQUIRED)
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio


# Add object file libraries
add_library(H8 h8.c h8.h v128.h minpos.h align.h)
add_library(Sort8 Sort8.cpp Sort8.hpp minpos.h)

# Add Executable
#add_executable(HeapAuxTest HeapAuxTest.cpp Heap8Aux.hpp 
#			         minpos.h v128.h align.h)
add_executable(HeapTest HeapTest.cpp StdMinHeap.hpp 
			         Heap8.hpp Heap8Aux.hpp H8.hpp 
			         minpos.h v128.h align.h h8.h)
#add_executable(H8Test h8Test.cpp minpos.h h8.h)
#add_executable(Sort8Test Sort8Test.cpp Sort8.hpp v128.h)
#add_executable(minposTest minposTest.cpp v128.h minpos.h)


# Link libraries
#TARGET_LINK_LIBRARIES(HeapAuxTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest_test) 
set (CMAKE_CXX_STANDARD 17)
TARGET_LINK_LIBRARIES(HeapTest LINK_PUBLIC ${Boost_LIBRARIES} gtest gtest_main H8) 
add_test(NAME example_test COMMAND HeapTest)
#TARGET_LINK_LIBRARIES(H8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest_test H8) 
#TARGET_LINK_LIBRARIES(Sort8Test LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest_test Sort8) 
#TARGET_LINK_LIBRARIES(minposTest LINK_PUBLIC ${Boost_LIBRARIES} gtest_main gtest_test) 

#add_executable(example example.cpp)
#target_link_libraries(example gtest gmock_main)
#add_test(NAME example_test COMMAND example)

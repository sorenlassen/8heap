CC=g++
OPT=-O2

all: runbenchmarks runtests

build: buildbenchmarks buildtests

runbenchmarks: buildbenchmarks
	./HeapBenchmark.out
	./Sort8Benchmark.out
	./minposBenchmark.out
	./HeapAuxBenchmark.out

buildbenchmarks: HeapBenchmark.out Sort8Benchmark.out minposBenchmark.out HeapAuxBenchmark.out

HeapBenchmark.out: HeapBenchmark.cpp StdMinHeap.hpp Heap8.hpp H8.hpp minpos.h v128.h align.h h8.h h8.o
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags h8.o HeapBenchmark.cpp -o HeapBenchmark.out

Sort8Benchmark.out: Sort8Benchmark.cpp Sort8.hpp Sort8.o
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags Sort8.o Sort8Benchmark.cpp -o Sort8Benchmark.out

minposBenchmark.out: minposBenchmark.cpp minpos.h
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lbenchmark minposBenchmark.cpp -o minposBenchmark.out

HeapAuxBenchmark.out: HeapAuxBenchmark.cpp Heap8Aux.hpp
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags  HeapAuxBenchmark.cpp -o HeapAuxBenchmark.out

runtests: buildtests
	./minposTest.out
	./Sort8Test.out
	./h8Test.out
	./HeapTest.out
	./HeapAuxTest.out

buildtests: HeapTest.out h8Test.out Sort8Test.out minposTest.out HeapTest.out HeapAuxTest.out

HeapAuxTest.out: HeapAuxTest.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp minpos.h v128.h align.h
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main HeapAuxTest.cpp -o HeapAuxTest.out

HeapTest.out: HeapTest.cpp StdMinHeap.hpp Heap8.hpp Heap8Aux.hpp H8.hpp minpos.h v128.h align.h h8.h h8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main h8.dbg.o HeapTest.cpp -o HeapTest.out

h8Test.out: h8Test.cpp minpos.h h8.h h8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main h8.dbg.o h8Test.cpp -o h8Test.out

Sort8Test.out: Sort8Test.cpp Sort8.hpp v128.h Sort8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main Sort8.dbg.o Sort8Test.cpp -o Sort8Test.out

minposTest.out: minposTest.cpp v128.h minpos.h
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main minposTest.cpp -o minposTest.out

h8.o: h8.c h8.h v128.h minpos.h align.h
	gcc -g -std=c11 -msse4 $(OPT) -DNDEBUG -c h8.c

h8.dbg.o: h8.c h8.h v128.h minpos.h align.h
	gcc -g -std=c11 -msse4 $(OPT) -c h8.c -o h8.dbg.o

Sort8.o: Sort8.cpp Sort8.hpp minpos.h
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -c Sort8.cpp

Sort8.dbg.o: Sort8.cpp Sort8.hpp minpos.h
	$(CC) -g -std=c++17 -msse4 $(OPT) -c Sort8.cpp -o Sort8.dbg.o

clean:
	rm -f *.o *.out
	rm -rf *.out.*

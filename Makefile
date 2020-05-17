CC=g++
OPT=-O2

all: runbenchmarks runtests

build: buildbenchmarks buildtests

runbenchmarks: buildbenchmarks
	./minposBenchmark.out
	./HeapBenchmark.out
	./HeapAuxBenchmark.out
	./Sort8Benchmark.out

buildbenchmarks: HeapBenchmark.out Sort8Benchmark.out minposBenchmark.out HeapAuxBenchmark.out

minposBenchmark.out: minposBenchmark.cpp minpos.h
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lbenchmark minposBenchmark.cpp -o minposBenchmark.out

HeapBenchmark.out: HeapBenchmark.cpp StdMinHeap.hpp Heap8.hpp H8.hpp minpos.h v128.h align.h h8.h h8.o
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags h8.o HeapBenchmark.cpp -o HeapBenchmark.out

HeapAuxBenchmark.out: HeapAuxBenchmark.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags  HeapAuxBenchmark.cpp -o HeapAuxBenchmark.out

Sort8Benchmark.out: Sort8Benchmark.cpp Sort8.hpp Sort8.o
	$(CC) -g -std=c++17 -msse4 $(OPT) -DNDEBUG -lfollybenchmark -lgflags Sort8.o Sort8Benchmark.cpp -o Sort8Benchmark.out

runtests: buildtests
	./minposTest.out
	./U48Test.out
	./h8Test.out
	./HeapTest.out
	./HeapAuxTest.out
	./Sort8Test.out

buildtests: minposTest.out U48Test.out h8Test.out HeapTest.out HeapAuxTest.out Sort8Test.out

U48Test.out: U48Test.cpp U48.hpp
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main U48Test.cpp -o U48Test.out

minposTest.out: minposTest.cpp v128.h minpos.h
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main minposTest.cpp -o minposTest.out

h8Test.out: h8Test.cpp minpos.h h8.h h8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main h8.dbg.o h8Test.cpp -o h8Test.out

HeapTest.out: HeapTest.cpp H8.hpp Heap8.hpp StdMinHeap.hpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h h8.h h8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main h8.dbg.o HeapTest.cpp -o HeapTest.out

HeapAuxTest.out: HeapAuxTest.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main HeapAuxTest.cpp -o HeapAuxTest.out

Sort8Test.out: Sort8Test.cpp Sort8.hpp v128.h Sort8.dbg.o
	$(CC) -g -std=c++17 -msse4 -lgtest -lgtest_main Sort8.dbg.o Sort8Test.cpp -o Sort8Test.out

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

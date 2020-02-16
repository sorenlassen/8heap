all: runbenchmarks runtests

build: buildbenchmarks buildtests

runbenchmarks: buildbenchmarks
	./HeapBenchmark.out
	./Sort8Benchmark.out
	./minposBenchmark.out

buildbenchmarks: HeapBenchmark.out Sort8Benchmark.out minposBenchmark.out

HeapBenchmark.out: HeapBenchmark.cpp StdMinHeap.hpp Heap8.hpp H8.hpp minpos.h v128.h align.h h8.h h8.o
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags h8.o HeapBenchmark.cpp -o HeapBenchmark.out

Sort8Benchmark.out: Sort8Benchmark.cpp Sort8.hpp Sort8.o
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags Sort8.o Sort8Benchmark.cpp -o Sort8Benchmark.out

minposBenchmark.out: minposBenchmark.cpp minpos.h
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark -lgflags minposBenchmark.cpp -o minposBenchmark.out

runtests: buildtests
	./minposTest.out
	./Sort8Test.out
	./h8Test.out
	./HeapTest.out

buildtests: HeapTest.out h8Test.out Sort8Test.out minposTest.out

HeapTest.out: HeapTest.cpp StdMinHeap.hpp HeapN.hpp Heap8.hpp H8.hpp minpos.h v128.h align.h h8.h h8.o
	g++ -g -std=c++17 -msse4 -lgtest -lgtest_main h8.o HeapTest.cpp -o HeapTest.out

h8Test.out: h8Test.cpp minpos.h h8.h h8.o
	g++ -g -std=c++14 -msse4 -lgtest -lgtest_main h8.o h8Test.cpp -o h8Test.out

Sort8Test.out: Sort8Test.cpp Sort8.hpp v128.h Sort8.o
	g++ -g -std=c++17 -msse4 -lgtest -lgtest_main Sort8.o Sort8Test.cpp -o Sort8Test.out

minposTest.out: minposTest.cpp v128.h minpos.h
	g++ -g -std=c++17 -msse4 -lgtest -lgtest_main minposTest.cpp -o minposTest.out

h8.o: h8.c h8.h v128.h minpos.h align.h
	gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c

Sort8.o: Sort8.cpp Sort8.hpp minpos.h
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -c Sort8.cpp

clean:
	rm -f *.o *.out
	rm -rf *.out.*

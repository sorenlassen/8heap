bench: HeapBenchmark.cpp StdMinHeap.hpp Heap8.hpp H8.hpp minpos.h h8.h h8.o
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o HeapBenchmark.cpp

h8.o: h8.c h8.h minpos.h
	gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c

clean:
	rm *.o a.out
	rm -r a.out.*

bench: h8.c HeapBenchmark.cpp
	gcc -g -std=c11 -msse4 -O2 -DNDEBUG -c h8.c
	g++ -g -std=c++17 -msse4 -O2 -DNDEBUG -lfollybenchmark h8.o HeapBenchmark.cpp

clean:
	rm *.o a.out
	rm -r a.out.*

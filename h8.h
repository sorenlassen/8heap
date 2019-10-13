#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // uint16_t

typedef uint16_t elem;

extern size_t heap_size();

// Increases heap by n consecutive element positions at the end and returns a
// pointer to the first of those positions.
//
// Note that this function breaks the heap invariant. After calling this
// function the caller must populate the new n positions at the end of the
// heap array and then call heapify on those n positions.
//
// Returns NULL if memory allocation fails,
extern elem* extend_heap(size_t n);

extern void pull_up(elem b, size_t q);

extern void push_down(elem a, size_t p);

extern void heapify(size_t skip);

extern bool push(elem b);

extern elem top();

extern elem pop();

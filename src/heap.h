/*
  A max heap supporting operations needed for kd-tree queries. Notably, the
  ability to build a heap from existing items is lacking.
*/
#ifndef _KATY_HEAP_H
#define _KATY_HEAP_H

struct MaxHeap {
  struct HeapItem **items;
  int size;
  int capacity;
};

struct HeapItem {
  void *item;
  double value;  // determines heap ordering
};

/*
  Create an empty max heap using heap memory. Client is responsible for freeing
  using free_max_heap().  Returns NULL on failure.
*/
struct MaxHeap *create_max_heap(int capactity);

/* Free a max heap and its underlying members. */
void free_max_heap(struct MaxHeap *heap);

/*
  Insert an item and its associated value into the heap.
  Returns `0` on failure to insert, `1` otherwise.
*/
int max_heap_insert(struct MaxHeap *heap, void *item, double value);

/*
  Get the item at the top of the heap and return it in `item` without removing
  it from the heap. Returns `0` on failure (empty heap), `1` otherwise
*/
int max_heap_peak(struct MaxHeap *heap, struct HeapItem **item);

/*
  Remove the item at the top of the heap and return it in `item`. Returns `0`
  on failure (empty heap), `1` otherwise.
*/
int max_heap_pop(struct MaxHeap *heap, struct HeapItem **item);

#endif  // _KATY_HEAP_H

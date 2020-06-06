/*
  A max heap supporting operations needed for a nearest
  neighbor search on a kd-tree.
*/

struct MaxHeap {
  struct HeapItem **items;
  int size;
  int capacity;
  // double (*comparator)(void *a, void *b, int k);
};

struct HeapItem {
  void *item;       // a k dimensional point
  double value;     // its distance, in an unknown metric
};

struct MaxHeap *create_max_heap(int capactity);
void free_max_heap(struct MaxHeap *heap);
void max_heap_insert(struct MaxHeap *heap, void *item, double value);
int max_heap_peak(struct MaxHeap *heap, struct HeapItem **item);
int max_heap_pop(struct MaxHeap *heap, struct HeapItem **return_item);

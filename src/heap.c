#include <stdlib.h>

#include "heap.h"


void max_heap_percolate_down();
void max_heap_percolate_up();

// heap sort
// void build_min_heap(double *arr, int n);
// double pop_heap(double *arr, int n);
// void percolate_down(double *arr, int index, int n);



/* sorts the array in ascending order inside [start, stop) */
// void heap_sort(double *arr, int start, int stop) {

//   int n = stop - start;

//   if (n < 2) {
//     return;
//   }

//   // copy the array, this is the heap
//   double *copy = malloc(sizeof(double) * n);
//   memcpy(copy, arr + start, sizeof(double) * n);

//   // build the heap by percolating down from the leaves -- floyd's
//   // bad cache behavior
//   build_min_heap(copy, n);

//   // pop and write to the array. this will be ascending order
//   for (int i = 0; i < n; i++) {
//     arr[start + i] = pop_heap(copy, n - i);
//   }

//   free(copy);
// }

// void build_min_heap(double *arr, int n) {
//   if (n < 2) {
//     return;
//   }
//   // starting above the leaves (trivially valid heaps),
//   // percolate each node down
//   for (int i = n / 2; i >= 0; i--) {
//     percolate_down(arr, i, n);
//   }
// }


struct MaxHeap *create_max_heap(int capacity) {
  struct MaxHeap *heap = malloc(sizeof(struct MaxHeap));
  if (heap == NULL) {
    return NULL;
  }

  heap->items = malloc(sizeof(struct HeapItem *) * capacity);
  if (heap->items == NULL) {
    free(heap);
    return NULL;
  }

  heap->capacity = capacity;
  heap->size = 0;
  return heap;
}

void free_max_heap(struct MaxHeap *heap) {
  for (int i = 0; i < heap->size; i++) {
    free(heap->items[i]);
  }
  free(heap->items);
  free(heap);
}


/*
  Insert the value and associated value into the heap,
  resiziing if necessary. Expected usage is tracking
  results in kd tree queries.
*/
void max_heap_insert(struct MaxHeap *heap, void *item, double value) {
  if (heap->size == heap->capacity) {
    // What is a good value here? I don't like a multiple
    // of the current capacity because I don't want to
    // overshoot far if the heap is big. On the other hand,
    // severe re-allocing by not increasing enough would
    // trash performance
    int new_cap = heap->capacity * 1.5;
    heap->items = realloc(heap->items, (sizeof(struct HeapItem *) * new_cap));
    heap->capacity = new_cap;
  }

  heap->items[heap->size] = malloc(sizeof(struct HeapItem));
  heap->items[heap->size]->item = item;
  heap->items[heap->size]->value = value;
  heap->size++;

  max_heap_percolate_up(heap, heap->size - 1);
}


/*
  Remove and return the top of the heap via the return parameter
  return_item. Returns 1 on success.
*/
int max_heap_pop(struct MaxHeap *heap, struct HeapItem **return_item) {

  if (heap->size == 0) {
    return 0;
  }

  *return_item = heap->items[0];

  // promote the last item
  heap->items[0] = heap->items[heap->size - 1];
  heap->size--;

  // sift down if necessary
  if (heap->size > 1) {
    max_heap_percolate_down(heap, 0);
  }

  return 1;
}

/*
  Get the item on the top of the heap. Returns 1 on success.
*/
int max_heap_peak(struct MaxHeap *heap, struct HeapItem **item) {
  if (heap->size == 0) {
    return 0;
  }
  *item = heap->items[0];
  return 1;
}

/* promote the heap item at index if its value is larger, then recurse. */
void max_heap_percolate_up(struct MaxHeap *heap, int index) {
  if (index == 0) return;

  int parent_index = (index - 1) / 2;
  if (heap->items[index]->value > heap->items[parent_index]->value) {
    struct HeapItem *tmp = heap->items[index];
    heap->items[index] = heap->items[parent_index];
    heap->items[parent_index] = tmp;

    max_heap_percolate_up(heap, parent_index);
  }
}

/* demote the heap item at index if its value is less, then recurse. */
void max_heap_percolate_down(struct MaxHeap *heap, int index) {
  int greatest = index;

  // leave in order or promote max of children and keep going
  int left_child = (2 * index) + 1;
  int right_child = (2 * index) + 2;

  if (left_child < heap->size) {
    if (heap->items[greatest]->value < heap->items[left_child]->value) {
      greatest = left_child;
    }
  }

  if (right_child < heap->size) {
    if (heap->items[greatest]->value < heap->items[right_child]->value) {
      greatest = right_child;
    }
  }

  if (greatest != index) {
    struct HeapItem *tmp = heap->items[greatest];
    heap->items[greatest] = heap->items[index];
    heap->items[index] = tmp;
    max_heap_percolate_down(heap, greatest);
  }
}


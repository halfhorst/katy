#include <stdlib.h>

#include "heap.h"

#define HEAP_RESIZE_FACTOR 1.25

/* promote the heap item at index if its value is larger, then recurse. */
void max_heap_percolate_up(struct MaxHeap *heap, int index);

/* demote the heap item at index if its value is less, then recurse. */
void max_heap_percolate_down(struct MaxHeap *heap, int index);


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

int max_heap_insert(struct MaxHeap *heap, void *item, double value) {
  if (heap->size == heap->capacity) {
    int new_cap = heap->capacity * HEAP_RESIZE_FACTOR;
    struct HeapItem **reallocated = realloc(heap->items,
                                            (sizeof(struct HeapItem *)
                                            * new_cap));
    if (reallocated == NULL) {
      return 0;
    }
    heap->items = reallocated;
    heap->capacity = new_cap;
  }
  heap->items[heap->size] = malloc(sizeof(struct HeapItem));
  heap->items[heap->size]->item = item;
  heap->items[heap->size]->value = value;
  heap->size++;

  max_heap_percolate_up(heap, heap->size - 1);
  return 1;
}

int max_heap_peak(struct MaxHeap *heap, struct HeapItem **item) {
  if (heap->size == 0) {
    return 0;
  }
  *item = heap->items[0];
  return 1;
}

int max_heap_pop(struct MaxHeap *heap, struct HeapItem **return_item) {
  if (heap->size == 0) {
    return 0;
  }
  *return_item = heap->items[0];

  heap->items[0] = heap->items[heap->size - 1];
  heap->size--;
  if (heap->size > 1) {
    max_heap_percolate_down(heap, 0);
  }

  return 1;
}



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

void max_heap_percolate_down(struct MaxHeap *heap, int index) {
  int greatest = index;

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


#include "gtest/gtest.h"

extern "C" {
  #include <stdlib.h>
  #include <stdio.h>
  #include "../heap.h"
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


TEST(TestHeap, Insert) {
  struct MaxHeap *heap = create_max_heap(10);
  EXPECT_EQ(heap->size, 0);

  char item[] = "test";
  double value = 100;
  max_heap_insert(heap, item, value);
  EXPECT_EQ(heap->size, 1);

  char item2[] = "bar";
  double value2 = 50;
  max_heap_insert(heap, item2, value2);
  EXPECT_EQ(heap->size, 2);

  free_max_heap(heap);
}

TEST(TestHeap, Peak) {
  struct MaxHeap *heap = create_max_heap(10);

  char item[] = "test";
  double bigger_value = 100;
  max_heap_insert(heap, item, bigger_value);

  struct HeapItem *peak_item;
  max_heap_peak(heap, &peak_item);
  EXPECT_EQ(peak_item->value, bigger_value);

  char item2[] = "bar";
  double smaller_value = 50;
  max_heap_insert(heap, item2, smaller_value);

  max_heap_peak(heap, &peak_item);
  EXPECT_EQ(peak_item->value, bigger_value);

  free_max_heap(heap);
}

TEST(TestHeap, Pop) {
  struct MaxHeap *heap = create_max_heap(10);

  char item[] = "test";
  double smaller_value = 50;
  max_heap_insert(heap, item, smaller_value);

  char item2[] = "bar";
  double bigger_value = 100;
  max_heap_insert(heap, item2, bigger_value);

  struct HeapItem *popped;
  max_heap_pop(heap, &popped);
  EXPECT_EQ(heap->size, 1);
  EXPECT_EQ(popped->value, 100);

  free_max_heap(heap);
}

TEST(TestHeap, InsertMany) {
  int n = 10000;
  struct MaxHeap *heap = create_max_heap(n);
  char item[] = "test";
  for (int i = 1; i <= n; i++) {
    max_heap_insert(heap, item, (double) i);
  }

  struct HeapItem *peak_item;
  max_heap_peak(heap, &peak_item);
  EXPECT_EQ(peak_item->value, n);
}

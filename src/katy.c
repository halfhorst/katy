#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "katy.h"
#include "heap.h"

// TODO: Re-arrange to work through return parameters so the client
//       can manage their own memory

struct KdNode *create_kd_node();
void recursive_free_kd_node(struct KdNode *node);
struct KdNode *recursive_select_median(double *points, int *indices, int num_indices, int k, int leaf_size);
int get_splitting_axis(double *points, int *indices, int num_indices, int k);
void partition_indices(double *points, int *indices, int num_indices, int k, int split_axis, int partition_index);
void swap(int *indices, int a, int b);
void recursive_nearest_neighbor_descent(struct KdTree *tree, struct KdNode *node, double *input,
                                        int n, struct MaxHeap *result_heap);
void recursive_query_range_descent(struct KdTree *tree, struct KdNode *node, double *test_point,
                                   double *radii, struct MaxHeap *result_heap);
double minkowski_1(double *a, double *b, int k);
double squared_minkowski_2(double *a, double *b, int k);

/*
  Create an empty kd-tree with dimensionality k.
  Returns NULL on failure to malloc.
*/
struct KdTree *create_kd_tree(int k) {
  struct KdTree *tree = malloc(sizeof(struct KdTree));
  if (tree == NULL) {
    return NULL;
  }
  tree->k = k;
  tree->root = NULL;
  tree->copied = false;
  tree->data = NULL;
  tree->size = 0;
  return tree;
}

/*
  Build a kd-tree from points provided. Input data is copied if copy_data is true.
  Returns NULL on failure.
*/
struct KdTree *build_kd_tree(double *input_points, int num_points, int k, int leaf_size, bool copy_data) {
  struct KdTree *tree = create_kd_tree(k);
  if (tree == NULL || num_points == 0) {
    return NULL;
  }

  // Will the tree hold a pointer to the data or a copy of the data it controls?
  if (copy_data) {
    double *points = malloc(sizeof(double) * num_points * k);
    if (points == NULL) {
      return NULL;
    }
    memcpy(points, points, sizeof(double) * num_points * k);
    tree->copied = true;
    tree->data = points;
  } else {
    tree->copied = false;
    tree->data = input_points;
  }
  tree->size = num_points;

  // setup an array for indexing the data
  int *indices = malloc(sizeof(int) * num_points);
  if (indices == NULL) {
    return NULL;
  }
  for (int i = 0; i < num_points; i++) indices[i] = i;

  // stop if we only have leaf-number of points
  if (num_points <= leaf_size) {
    struct KdNode *node = malloc(sizeof(struct KdNode));
    if (node == NULL) {
      return NULL;
    }
    node->is_leaf = true;
    node->indices = indices;
    node->num_indices = num_points;
    node->low = NULL;
    node->high = NULL;

    tree->root = node;
    return tree;
  }

  // operate on indices into the data array
  tree->root = recursive_select_median(tree->data, indices, num_points, k, leaf_size);
  return tree;
}

/* Free a kd tree and its accompanying data */
void free_kd_tree(struct KdTree *tree) {
  if (tree->root != NULL) {
    recursive_free_kd_node(tree->root);
  }

  if(tree->copied) {
    free(tree->data);
  }
  free(tree);
}

/* Free a kd tree node and its children recursively */
void recursive_free_kd_node(struct KdNode *node) {
  // node guaranteed not null
  if (node->low != NULL) {
    recursive_free_kd_node(node->low);
  }
  if (node->high != NULL) {
    recursive_free_kd_node(node->high);
  }
  free(node->indices);
  free(node);
}

struct KdNode *recursive_select_median(double *points, int *indices, int num_indices, int k, int leaf_size) {

  int *indices_copy = malloc(sizeof(int) * num_indices);
  for (int i = 0; i < num_indices; i++) {
    indices_copy[i] = indices[i];
  }

  // if we are at num_indices < leaf_size, make a leaf and return it
  if (num_indices <= leaf_size) {
    struct KdNode *node = malloc(sizeof(struct KdNode));
    if (node == NULL) {
      return NULL;
    }
    node->is_leaf = true;
    node->indices = indices_copy;
    node->num_indices = num_indices;
    node->low = NULL;
    node->high = NULL;
    return node;
  }

  int splitting_axis = get_splitting_axis(points, indices, num_indices, k);

  // partition around the middle of the array
  int median_index = num_indices / 2;
  partition_indices(points, indices, num_indices, k, splitting_axis, median_index);

  // make a node
  struct KdNode *node = malloc(sizeof(struct KdNode));
  if (node == NULL) {
    return NULL;
  }
  node->is_leaf = false;
  node->indices = indices_copy;      // TODO: Only need this copy for testing
  node->num_indices = num_indices;
  node->split_axis = splitting_axis;
  node->split_value = points[(indices[median_index] * k) + splitting_axis];

  // here we pass in points to the beginning and the middle of the index array and
  // adjust the count of indices accordingly
  node->low = recursive_select_median(points, indices, median_index, k, leaf_size);
  node->high = recursive_select_median(points, indices + median_index, num_indices - median_index, k, leaf_size);

  return node;
}


/* Determine the splitting axis based on maximum spread. */
int get_splitting_axis(double *points, int *indices, int num_indices, int k) {
  int minimums[k];
  int maximums[k];
  for (int i = 0; i < k; i++) {
    minimums[i] = points[i];
    maximums[i] = points[i];
  }

  for (int i = 1; i < num_indices; i++) {
    for (int j = 0; j < k; j++) {
      double value = points[(indices[i] * k) + j];
      if (value < minimums[j]) {
        minimums[j] = value;
      } else if (value > maximums[j]) {
        maximums[j] = value;
      }
    }
  }

  double max_spread = 0;
  int split_axis = -1;
  for (int i = 0; i < k; i++) {
    double spread = maximums[i] - minimums[i];
    if (spread > max_spread) {
      max_spread = spread;
      split_axis = i;
    }
  }

  return split_axis;
}


/* A quicksort-esque partition around split_index. Smaller values precede the parition_index.*/
void partition_indices(double *points, int *indices, int num_indices, int k, int split_axis, int partition_index) {

  int left = 0;
  int right = num_indices - 1;

  int middle;
  while(true) {
    middle = left;
    for (int i = left; i < right; i++) {
      double val1 = points[indices[i] * k + split_axis];
      double val2 = points[indices[right] * k + split_axis];
      if (val1 < val2) {
        swap(indices, i, middle);
        middle++;
      }
    }
    swap(indices, middle, right);
    if (middle == partition_index) {
      break;
    } else if (middle < partition_index) {
      left = middle + 1;
    } else {
      right = middle - 1;
    }
  }
}

void swap(int *indices, int a, int b) {
  int tmp = indices[a];
  indices[a] = indices[b];
  indices[b] = tmp;
}

int kd_tree_query_n_nearest_neighbors(struct KdTree *tree, double *input,
                                      int n, char *distance_metric,
                                      struct KdResult **results) {

  // find n closest to input point
  // use a max heap to keep track, peek the top for the greatest distance
  // if greater than pop and push the new point

  // at the end make the result by poping and putting into results

  if (tree->size == 0) {
    return 0;
  }

  struct MaxHeap *results_heap;
  // TODO: different distance metrics
  if (strncmp(distance_metric, "euclidean", 9)) {
    results_heap = create_max_heap(n);
  } else if (strncmp(distance_metric, "manhattan", 9)) {
    results_heap = create_max_heap(n);
  } else {
    results_heap = create_max_heap(n);
  }

  recursive_nearest_neighbor_descent(tree, tree->root, input, n, results_heap);

  struct HeapItem *item;
  int num_results = results_heap->size;
  *results = malloc(sizeof(struct KdResult) * num_results);
  for (int i = 0; i < num_results; i++) {
    max_heap_pop(results_heap, &item);
    (*results)[i].point = item->item;
    (*results)[i].distance = item->value;
  }

  free_max_heap(results_heap);

  return num_results;
}

void recursive_nearest_neighbor_descent(struct KdTree *tree, struct KdNode *node, double *test_point, int n,
                                        struct MaxHeap *result_heap) {
  if (node == NULL) {
    return;
  }

  if (node->is_leaf) {
    // check all points in this leaf node
    // stick in the heap if it isn't full or if its closer than
    // the furthest already in the heap
    for (int i = 0; i < node->num_indices; i++) {
      double *point = tree->data + (node->indices[i] * tree->k);
      double distance = squared_minkowski_2(point, test_point, tree->k);
      if (result_heap->size < n) {
        max_heap_insert(result_heap, point, distance);
      } else {
        struct HeapItem *top_item;
        max_heap_peak(result_heap, &top_item);
        if (top_item->value > distance) {
          struct HeapItem *throwaway;
          max_heap_pop(result_heap, &throwaway);
          max_heap_insert(result_heap, point, distance);
        }
      }
    }
    return;
  }

  // descend on the same side as the test point
  bool took_low = false;
  if (test_point[node->split_axis] < node->split_value) {
  // recur on the lower side
    recursive_nearest_neighbor_descent(tree, node->low, test_point, n, result_heap);
    took_low = true;
  } else {
    // recrur on the higher side
    recursive_nearest_neighbor_descent(tree, node->high, test_point, n, result_heap);
  }

  // now decide if the other side is a possibility
  struct HeapItem *current_furthest;
  max_heap_peak(result_heap, &current_furthest);
  double max_distance = current_furthest->value;
  if (took_low) {
    if ((test_point[node->split_axis] + max_distance) >= node->split_value) {
      recursive_nearest_neighbor_descent(tree, node->high, test_point, n, result_heap);
    }
  } else {
    if ((test_point[node->split_axis] - max_distance) <= node->split_value) {
      recursive_nearest_neighbor_descent(tree, node->low, test_point, n, result_heap);
    }
  }
}

int kd_tree_query_range(struct KdTree *tree, double *test_point, double *radii,
                        char *distance_metric, struct KdResult **results) {
  if (tree->size == 0) {
    return 0;
  }

  struct MaxHeap *results_heap;
  int initial_heap_capacity = 100;
  // TODO: different distance metrics
  if (strncmp(distance_metric, "euclidean", 9)) {
    results_heap = create_max_heap(initial_heap_capacity);
  } else if (strncmp(distance_metric, "manhattan", 9)) {
    results_heap = create_max_heap(initial_heap_capacity);
  } else {
    results_heap = create_max_heap(initial_heap_capacity);
  }

  recursive_query_range_descent(tree, tree->root, test_point, radii, results_heap);

  struct HeapItem *item;
  int num_results = results_heap->size;
  *results = malloc(sizeof(struct KdResult) * num_results);
  for (int i = 0; i < num_results; i++) {
    max_heap_pop(results_heap, &item);
    (*results)[i].point = item->item;
    (*results)[i].distance = item->value;
  }

  free_max_heap(results_heap);

  return num_results;
}


void recursive_query_range_descent(struct KdTree *tree, struct KdNode *node, double *test_point,
                                   double *radii, struct MaxHeap *result_heap) {

  if (node == NULL) {
    return;
  }

  if (node->is_leaf == true) {
    for (int i = 0; i < node->num_indices; i++) {
      // check the distance between test point and node point
      // is it less than radii?
      double *point = tree->data + (node->indices[i] * tree->k);
      bool inside = true;
      for (int j = 0; j < tree->k; j++) {
        if (fabs(point[j] - test_point[j]) > radii[j]) {
          inside = false;
        }
      }
      if (inside) {
        double distance = squared_minkowski_2(point, test_point, tree->k);
        max_heap_insert(result_heap, point, distance);
      }
    }
    return;
  }

  if ((test_point[node->split_axis] + radii[node->split_axis]) >= node->split_value) {
    recursive_query_range_descent(tree, node->high, test_point, radii, result_heap);
  }
  if ((test_point[node->split_axis] - radii[node->split_axis]) <= node->split_value) {
    recursive_query_range_descent(tree, node->low, test_point, radii, result_heap);
  }
}

/* manhattan distance */
double minkowski_1(double *a, double *b, int k) {
  double dist = 0;
  for (int i = 0; i < k; i++) {
    dist += fabs(a[i] - b[i]);
  }
  return dist;
}

/* euclidean distance */
double squared_minkowski_2(double *a, double *b, int k) {
  double dist = 0;
  for (int i = 0; i < k; i++) {
    dist += pow(a[i] - b[i], 2);
  }
  return dist;
}

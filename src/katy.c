#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "katy.h"
#include "heap.h"

void recursive_free_kd_node(struct KdNode *node);

/*
  Select the median of the longest axis from among the points in the set of
  indices, then recursively call to determine the low and high subtree. Returns
  a node representing the split, or NULL on failure.
*/
struct KdNode *recursive_select_median(double *points, int *indices,
                                       int num_indices, int k, int leaf_size);

/*
  Determine the axis of greatest spread from among the points in the set of
  indices.
*/
int get_splitting_axis(double *points, int *indices, int num_indices, int k);

/*
  Partition the indices array in-place based on values from the points array
  into smaller values on the split_axis below the partition_index and greater
  values above.
*/
void partition_indices(double *points, int *indices, int num_indices, int k,
                       int split_axis, int partition_index);

/* Utility function for swapping elements of the index array. */
void swap(int *indices, int a, int b);

/*
  Ridiculous function pointer syntax. get_comparison_function parses
  distance_metric and returns a pointer to a function with a signature like so:

  double fxn(double *a, double *b, int k) -- a.k.a a distance function
*/
double (*get_distance_function(char *distance_metric))(double *a, double *b,
                                                       int k);

/*
  Recursively descend down the kd-tree, pushing points onto the result_heap
  if less than `n` are currently recorded or their distance is less than the
  current maximum.
*/
void recursive_nearest_neighbor_descent(struct KdTree *tree,
                                        struct KdNode *node, double *input,
                                        int n, struct MaxHeap *result_heap,
                                        double (*distance_function)(double *a,
                                                                    double *b,
                                                                    int k));

/*
  Recursively descend down the kd-tree, pushing points onto the result_heap if
  they lie within the `radii` around the `test_point`.
*/
void recursive_query_range_descent(struct KdTree *tree, struct KdNode *node,
                                   double *test_point, double *radii,
                                   struct MaxHeap *result_heap,
                                   double (distance_function)(double *a,
                                                              double *b,
                                                              int k));

/* Minkowski distance where p = 1, a.k.a. Manhattan distance. */
double minkowski_1(double *a, double *b, int k);

/*
  Squared Minkowski distance where p = 2, a.k.a. Squared Euclidean distance.
  Useful because it avoids a root operation.
 */
double squared_minkowski_2(double *a, double *b, int k);


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

struct KdTree *build_kd_tree(double *input_points, int num_points, int k,
                             int leaf_size, bool copy_data) {
  struct KdTree *tree = create_kd_tree(k);
  if (tree == NULL || num_points == 0) {
    return NULL;
  }

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

  int *indices = malloc(sizeof(int) * num_points);
  if (indices == NULL) {
    return NULL;
  }
  for (int i = 0; i < num_points; i++) indices[i] = i;

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

  tree->root = recursive_select_median(tree->data, indices, num_points, k,
                                       leaf_size);
  return tree;
}

void free_kd_tree(struct KdTree *tree) {
  if (tree->root != NULL) {
    recursive_free_kd_node(tree->root);
  }

  if (tree->copied) {
    free(tree->data);
  }
  free(tree);
}

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

struct KdNode *recursive_select_median(double *points, int *indices,
                                       int num_indices, int k, int leaf_size) {
  int *indices_copy = malloc(sizeof(int) * num_indices);
  for (int i = 0; i < num_indices; i++) {
    indices_copy[i] = indices[i];
  }

  // bail if we have less than leaf number of points
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
  if (splitting_axis == -1) {
    return NULL;
  }

  int median_index = num_indices / 2;
  partition_indices(points, indices, num_indices, k, splitting_axis,
                    median_index);

  struct KdNode *node = malloc(sizeof(struct KdNode));
  if (node == NULL) {
    return NULL;
  }
  node->is_leaf = false;
  node->indices = indices_copy;
  node->num_indices = num_indices;
  node->split_axis = splitting_axis;
  node->split_value = points[(indices[median_index] * k) + splitting_axis];

  // Continue on, selecting medians among the two sets of points partitioned
  // about the median
  node->low = recursive_select_median(points, indices, median_index, k,
                                      leaf_size);
  node->high = recursive_select_median(points, indices + median_index,
                                       num_indices - median_index, k,
                                       leaf_size);

  return node;
}


/*
  Track the minimum and maximum of each dimension in one traversal of the
  points, Then determine the largest spread among the dimensions by difference.
*/
int get_splitting_axis(double *points, int *indices, int num_indices, int k) {
  int *minimums = malloc(sizeof(int) * k);
  int *maximums = malloc(sizeof(int) * k);

  if ((minimums == NULL) | (maximums == NULL)) {
    return -1;
  }

  // use the first point to pre-populate
  for (int i = 0; i < k; i++) {
    minimums[i] = points[(indices[0] * k) + i];
    maximums[i] = points[(indices[0] * k) + i];
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

  free(minimums);
  free(maximums);
  return split_axis;
}

void partition_indices(double *points, int *indices, int num_indices, int k,
                       int split_axis, int partition_index) {
  int left = 0;
  int right = num_indices - 1;

  int middle;
  while (true) {
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


double (*get_distance_function(char *distance_metric))(double *a, double *b,
                                                       int k) {
  if (strncmp(distance_metric, "squared_euclidean", 17) == 0) {
    return squared_minkowski_2;
  } else if (strncmp(distance_metric, "manhattan", 9) == 0) {
    return minkowski_1;
  } else {
    fprintf(stderr, "Unknown distance metric encountered.\n");
    exit(EXIT_FAILURE);
  }
}

int kd_tree_query_n_nearest_neighbors(struct KdTree *tree, double *input,
                                      int n, char *distance_metric,
                                      struct KdResult **results) {
  if (tree->size == 0) {
    return 0;
  }

  struct MaxHeap *results_heap = create_max_heap(n);


  recursive_nearest_neighbor_descent(tree, tree->root, input, n, results_heap,
                                     get_distance_function(distance_metric));

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

/*
  Descends down the tree recursively, selecting regions that contain the
  test point. We determine if the other side of the splitting plane could
  contain a point loser, and check it if so.
*/
void recursive_nearest_neighbor_descent(struct KdTree *tree,
                                        struct KdNode *node,
                                        double *test_point, int n,
                                        struct MaxHeap *result_heap,
                                        double (*distance_function)(double *a,
                                                                    double *b,
                                                                    int k)) {
  if (node == NULL) {
    return;
  }

  if (node->is_leaf) {
    for (int i = 0; i < node->num_indices; i++) {
      double *point = tree->data + (node->indices[i] * tree->k);
      double distance = distance_function(point, test_point, tree->k);
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
    recursive_nearest_neighbor_descent(tree, node->low, test_point, n,
                                       result_heap, distance_function);
    took_low = true;
  } else {
    recursive_nearest_neighbor_descent(tree, node->high, test_point, n,
                                       result_heap, distance_function);
  }

  // Decide if the other side of the splitting plane is a possibility
  struct HeapItem *current_furthest;
  max_heap_peak(result_heap, &current_furthest);
  double max_distance = current_furthest->value;
  if (took_low) {
    if ((test_point[node->split_axis] + max_distance) >= node->split_value) {
      recursive_nearest_neighbor_descent(tree, node->high, test_point, n,
                                         result_heap, distance_function);
    }
  } else {
    if ((test_point[node->split_axis] - max_distance) <= node->split_value) {
      recursive_nearest_neighbor_descent(tree, node->low, test_point, n,
                                         result_heap, distance_function);
    }
  }
}

int kd_tree_query_range(struct KdTree *tree, double *test_point, double *radii,
                        char *distance_metric, struct KdResult **results) {
  if (tree->size == 0) {
    return 0;
  }

  int initial_heap_capacity = 100;
  struct MaxHeap *results_heap = create_max_heap(initial_heap_capacity);
  recursive_query_range_descent(tree, tree->root, test_point, radii,
                                results_heap,
                                get_distance_function(distance_metric));

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

/*
  Recursively descend down the tree, only entering regions that intersect the
  query range area.
*/
void recursive_query_range_descent(struct KdTree *tree, struct KdNode *node,
                                   double *test_point, double *radii,
                                   struct MaxHeap *result_heap,
                                   double (*distance_function)(double *a,
                                                               double *b,
                                                               int k)) {
  if (node == NULL) {
    return;
  }

  if (node->is_leaf) {
    for (int i = 0; i < node->num_indices; i++) {
      // check the distance between test point and kd-tree point in each
      // dimension to determine if it satisfies the range query.
      double *point = tree->data + (node->indices[i] * tree->k);
      bool inside = true;
      for (int j = 0; j < tree->k; j++) {
        if (fabs(point[j] - test_point[j]) > radii[j]) {
          inside = false;
        }
      }
      if (inside) {
        double distance = distance_function(point, test_point, tree->k);
        max_heap_insert(result_heap, point, distance);
      }
    }
    return;
  }

  if ((test_point[node->split_axis] + radii[node->split_axis])
      >= node->split_value) {
    recursive_query_range_descent(tree, node->high, test_point, radii,
                                  result_heap, distance_function);
  }
  if ((test_point[node->split_axis] - radii[node->split_axis])
      <= node->split_value) {
    recursive_query_range_descent(tree, node->low, test_point, radii,
                                  result_heap, distance_function);
  }
}

double minkowski_1(double *a, double *b, int k) {
  double dist = 0;
  for (int i = 0; i < k; i++) {
    dist += fabs(a[i] - b[i]);
  }
  return dist;
}

double squared_minkowski_2(double *a, double *b, int k) {
  double dist = 0;
  for (int i = 0; i < k; i++) {
    dist += pow(a[i] - b[i], 2);
  }
  return dist;
}

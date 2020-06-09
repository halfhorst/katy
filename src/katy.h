/*
  A kd-tree supporting k-dimensional points composed of doubles and
  n-nearest-neighbor and range queries. The tree contains a pointer to the
  underlying data (optionally copied) and stores indices into that data in
  leaves in the tree.

  The tree is built using by splitting at the median along the longest axis at
  each level of the tree. The median is selected using a quicksort-esque pivot,
  drawn from the sklearn implementation.

  Katy does not support insertion or deletion, which can degenerate a kd-tree.
  To alter the points contained, rebuild the tree.
*/
#ifndef _KATY_H_
#define _KATY_H_

#include <stdbool.h>

struct KdNode {
  struct KdNode *low;   // The subtree containins points lesser than split
                        // value along the split axis.
  struct KdNode *high;  // The subtree containing points greater than split
                        // value along the split axis.
  int *indices;         // Indices of all data below this point in the tree.
  double split_value;   // The demarcating value along the split axis
  int split_axis;       // The axis along which this node represents a split
  int num_indices;      // The number of indices contained if a leaf.
  bool is_leaf;         // Leaf nodes do not represent splits
};

struct KdTree {
  struct KdNode *root;
  double *data;
  int size;
  int k;
  bool copied;          // Was the input data copied?
};

/* A query result, containing a k dimensional point and a distance. */
struct KdResult {
  double *point;
  double distance;
};

/* Create an empty kd-tree with dimensionality k. Returns NULL on failure. */
struct KdTree *create_kd_tree(int k);

/*
  Build a kd-tree from an array of points. `leaf_size` dictates the threshold
  number of points at which splitting stops and leaf node ism ade. Input data
  is copied if `copy_data` is true. Returns NULL on failure.
*/
struct KdTree *build_kd_tree(double *points, int num_points, int k,
                             int leaf_size, bool copy_data);

/* Free a kd tree and its underlying data if copied. */
void free_kd_tree(struct KdTree *tree);

/*
  Find the `n` nearest neighbors to the `test_point` according to a specific
  distance metric. Results are returned through the `results` return parameter,
  which is an array of KdResult. The actual number of neighbors found is
  returned by the function.

  Acceptable distance metrics are `manhattan` and `squared_euclidean`.
*/
int kd_tree_query_n_nearest_neighbors(struct KdTree *tree, double *test_point,
                                      int n, char *distance_metric,
                                      struct KdResult **results);

/*
  Find all points that lie within a specific range of the `test_point`. The
  range is specified by a k-dimensional point of radii assumed to be symmetric
  around the test point. For example, in 2d space, if radii is specified as
  (5.0, 5.0), then the query range encompases (x +/- 5, y +/- 5).

  Results are returned through the `results` return parameter, which is an
  array of KdResult. The actual numver of points found is returned by the
  function.
*/
int kd_tree_query_range(struct KdTree *tree, double *test_point, double *radii,
                        char *distance_metric, struct KdResult **results);

#endif  // _KATY_H_

#ifndef _KATY_H_
#define _KATY_H_

struct KdNode {
  struct KdNode *low;   // malloc'd memory. Te less than subtree
  struct KdNode *high;  // malloc'd memory. The greater than subtree
  int *indices;         // malloc'd memory. indices of points into the tree's data if this node is a leaf
  double split_value;   // The demarcating value along the split axis
  int split_axis;       // The axis along which this node represents a split
  int num_indices;      // The number of indices contained if a leaf.
  bool is_leaf;         // Is this node a leaf? Maybe don't need
};

struct KdTree {
  struct KdNode *root;  // The tree's root node
  double *data;         // A pointer to the tree's data. Malloc'd and owned if copied is true
  int size;             // The number of points in the tree. This is static
  int k;                // The dimensionality of points in the tree
  bool copied;          // Was the input data copied?
};

/* A query result, containing a k dimensional point and a distance. */
struct KdResult {
  double *point;
  double distance;
};

struct KdTree *create_kd_tree(int k);
struct KdTree *build_kd_tree(double *points, int num_points, int k,
                             int leaf_size, bool copy_data);
void free_kd_tree(struct KdTree *tree);

/*
  Query routines.
  We support range-based queries and nearest neighbors queries.
*/

int kd_tree_query_n_nearest_neighbors(struct KdTree *tree, double *test_point,
                                      int n, char *distance_metric,
                                      struct KdResult **results);
int kd_tree_query_range(struct KdTree *tree, double *test_point, double *radii,
                        char *distance_metric, struct KdResult **results);

#endif  // _KATY_H_

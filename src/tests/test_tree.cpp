#include "gtest/gtest.h"

extern "C" {
  #include <stdlib.h>
  #include <stdio.h>
  #include "../katy.h"
  #include "../heap.h"
}

void random_nonzero_array(double *arr, int n, int range);
void check_tree_invariant(struct KdTree *tree);
void recursive_check_node_invariant(struct KdTree *tree, struct KdNode *node);

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestBuildTree, BuildEmptyTreeVaryingDimensions) {
  for (int i = 0; i < 20; i++) {
    struct KdTree *tree = create_kd_tree(i);
    EXPECT_FALSE(tree == NULL);
    free_kd_tree(tree);
  }
}

TEST(TestBuildTree, CopyData) {
  double points[10];
  random_nonzero_array(points, 10, 40);
  struct KdTree *tree = build_kd_tree(points, 5, 2, 20, true);
  EXPECT_NE(tree->data, points);
}

TEST(TestBuildTree, PreserveData) {
  double points[10];
  random_nonzero_array(points, 10, 40);
  struct KdTree *tree = build_kd_tree(points, 5, 2, 20, false);
  EXPECT_EQ(tree->data, points);
}

TEST(TestBuildTree, BuildLessThanLeafPoints) {
  double points[10];
  random_nonzero_array(points, 10, 40);
  struct KdTree *tree = build_kd_tree(points, 5, 2, 20, false);
  EXPECT_TRUE(tree->root->is_leaf);
  EXPECT_EQ(tree->root->low, nullptr);
  EXPECT_EQ(tree->root->high, nullptr);
  check_tree_invariant(tree);
}

TEST(TestBuildTree, FewPoints) {
  int leaf_size = 1;
  int size = 10;
  int k = 2;
  double points[20];
  random_nonzero_array(points, size * k, 40);
  struct KdTree *tree = build_kd_tree(points, size, k, leaf_size, false);
  EXPECT_NE(tree->root, nullptr);
  EXPECT_FALSE(tree->root->is_leaf);
  check_tree_invariant(tree);
}

TEST(TestBuildTree, ManyPoints) {
  double points[10000];
  random_nonzero_array(points, 10000, 10000);
  struct KdTree *tree = build_kd_tree(points, 5000, 2, 1, false);
  EXPECT_NE(tree->root, nullptr);
  EXPECT_FALSE(tree->root->is_leaf);
  check_tree_invariant(tree);
}

TEST(TestQuery, NearestNeighbor) {
  // a 10x10 cube
  double points[] = {0.0, 0.0, 10.0, 10.0, 10.0, 0.0, 0.0, 10.0};
  int num_points = 4;
  int k = 2;
  int leaf_size = 1;
  bool copy_data = false;
  struct KdTree *tree = build_kd_tree(points, num_points, k, leaf_size, copy_data);

  double test_point[] = {9.0, 9.0};  // expect (10, 10)
  int n = 1;
  char distance[] = "squared_euclidean";
  struct KdResult *results;
  int num_results = kd_tree_query_n_nearest_neighbors(tree, test_point, n, distance, &results);
  EXPECT_EQ(num_results, n);
  EXPECT_EQ(results[0].point[0], 10);
  EXPECT_EQ(results[0].point[1], 10);
  EXPECT_EQ(results[0].distance, 2);
}

TEST(TestQuery, RangeSearch) {
  // a 10x10 cube
  double points[] = {0.0, 0.0, 10.0, 10.0, 10.0, 0.0, 0.0, 10.0};
  int num_points = 4;
  int k = 2;
  int leaf_size = 1;
  bool copy_data = false;
  struct KdTree *tree = build_kd_tree(points, num_points, k, leaf_size, copy_data);

  double test_point[] = {5.0, 10.0};
  double radii[] = {6.0, 1.0};
  char distance[] = "squared_euclidean";
  struct KdResult *results;
  int num_results = kd_tree_query_range(tree, test_point, radii, distance, &results);
  EXPECT_EQ(num_results, 2);  // (0, 10), (10, 10);
}

void random_nonzero_array(double *arr, int n, int range) {
  for (int i = 0; i < n; i++) {
    double val = rand();  // srand(1) default
    if (val == 0) val = 1;
    arr[i] = rand() % range;
  }
}

void check_tree_invariant(struct KdTree *tree) {
  if (tree->root == NULL) {
    return;
  }
  recursive_check_node_invariant(tree, tree->root);
}

void recursive_check_node_invariant(struct KdTree *tree, struct KdNode *node) {
  if (!node->is_leaf) {

    for (int i = 0; i < node->low->num_indices; i++) {
      int data_index = (node->low->indices[i] * tree->k) + node->split_axis;
      EXPECT_LE(tree->data[data_index], node->split_value);
    }

    for (int i = 0; i < node->high->num_indices; i++) {
      int data_index = (node->high->indices[i] * tree->k) + node->split_axis;
      EXPECT_GE(tree->data[data_index], node->split_value);
    }
    recursive_check_node_invariant(tree, node->low);
    recursive_check_node_invariant(tree, node->high);
  }
}

# Katy: A kd-tree C library and python C extension

kd-trees are one of many data structures suitable for answering geometrics queries. Specifically, they are good at answering
nearest neighbor queries and range queries.

Kd trees themselves have many variants, generally revolving around the splitting rule used to generate the splitting hyperplanes. Median-finding is another source of variants, as many splitting methods rely on finding the median of a set of points and the sorts and methods used vary in complexity profile. This represents a balance between upfront building time generating a tree that handles queries faster.

The original paper simply cycled through axes, which was followed up by selecting the axis with the greates spread and splitting it at the median, which generates boxes of a bounded aspect ratio that may also be empty. An improvement over both of these is known as the sliding midpoint rule. That is implemented here, and is also used in scipy's implementation.

This structure suffers from the curse of dimensionality -- as dimensionality increases, runtime tends towards visiting all points, e.g. brute force. An adaptation is to specify a leaf size, which refers to a threshold number of points below which to stop splitting axes and create a leaf in the tree. When answering queries, this is where the search becomes brute force.

## Implementation

Katy splits using the median of the axis with the largest spread at each level. It finds the median using a quick select method similar to the partition routine of quicksort, inspired by sklearn's implementation. It stores all values in the leaves.

Katy does not support insertion or deletion. Since no good balancing method exists (for a vanilla tree), insertions and deletions lead to suboptimal trees. If you want to change change the points in the tree, build a new tree.

Katy supports `n` nearest neighbor searches and range searches from a test point. The range searches are additionally specified with an array of radii for each axis in `k`.

# Other Things to Explore:

* A ball tree, suitable for higher dimensions
* A quad/octree
* BVH and AABB as it pertains to rendering
* Spacefilling curves


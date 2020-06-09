# Katy: A kd-tree C library and python C extension

This library is a personal project, conceived of for practice with a spatial
data structure and digging into Cpython. There are far better kd-tree
implementations in the wild, and this one is liable to WIP on the master branch
anyways.

kd-trees are tree structures containing k-dimensional points that partition the
points along one dimension at each level of the tree. There is considerable
variety around this theme in different implementations that revolve around how
best to select and split the points, and how to balance build time and query
time. These trees are suitable for answering geometric queries like "give me
the n closest points to this one" (k nearest neighbors) or "give me all the
points that lie this close to this point" (range queries). Note that closeness
here is ambiguous and requires definition.

The original paper by J.L Bentley [1] that proposed kd-trees simply cycled
through axes in order and selected a partition value that was the median of the
points along that axis. This was followed by a paper [2] that selected the axis
with the greatest spread at each level of the tree. Later, a midpoint rule was
proposed that selects the partition value based not on the median, but the
midpoint. Significantly, this means that spread out data, after the first
split, can lead to a subsequent split that is degenerate, e.g. all points lie
on one side. This is ameliorated by sliding the partition so that each side
captures at least one point, known as the sliding-midpoint rule [3]. There are
many other variations out there.

Using the median vs. the midpoint has consequences on the build time and
geometry of the tree structure. Finding the median requires a partial
sort or median selection, whereas the sliding midpoint requires a full sort to
determine if the split is degenerate. The midpoint results in trees that have
regions of bounded aspect ratio, but the tree itself may be unbalanced, which
could rear its head during some queries. The
median, on the other hand, may generate extreme aspect ratios but the tree
height is bounded. This is because the median is splitting the tree into equal
regions by _number of points_ whereas the midpoint is splitting the tree into
equal regions by _value along the axis_.

This structure suffers from the curse of dimensionality -- as `k` increases,
your queres will slowdown as the algorithm tends towards visiting all points,
e.g. a brute force solution. Specifying a leaf size can help, which dictates a
threshold number of points below which to stop splitting axes and create a leaf
in the tree.

[1] Bentley, Jon Louis. "Multidimensional binary search trees used for
    associative searching." Communications of the ACM 18.9 (1975): 509-517.
[2] Friedman, Jerome H., Jon Louis Bentley, and Raphael Ari Finkel. "An
    algorithm for finding best matches in logarithmic expected time." ACM
    Transactions on Mathematical Software (TOMS) 3.3 (1977): 209-226.
[3] Maneewongvatana, Songrit, and David M. Mount. "It’s okay to be skinny, if
    your friends are fat." Center for Geometric Computing 4th Annual Workshop
    on Computational Geometry. Vol. 2. 1999.

## Implementation

Katy splits using the median of the axis with the largest spread at each level.
It finds the median using a quick select method similar to the partition
routine of quicksort, inspired by sklearn's implementation. It stores indices
into the data at the leaves, and also all indices in the subtree at each node.

Katy does not support insertion or deletion. Since no good balancing method
exists (for a vanilla kd-tree), insertions and deletions lead to degenerate
trees. If you want to change change the points in the tree, build a new tree.

Katy supports `n` nearest neighbor searches and range searches from a test
point. The range searches are additionally specified with an array of radii for
each axis in `k`.

## Building / Running

Currently, the Makefile only supports building and running (sparse) tests. Note
that the tests require googletest, and expect it to be installed so your
compiler and linker can find what they need.

## What's next

* More extensive testing
* _The actual Python binding_

# Other Things to Explore:

* A ball tree, suitable for higher dimensions
* A quad/octree
* BVH and AABB as it pertains to rendering
* Spacefilling curves

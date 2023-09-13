# datrie

A C implementation of a double-array trie with suffix compression and UTF-8 support.

A trie, also called digital tree or prefix tree, is a type of k-ary search tree, a tree data structure used for locating specific keys from within a set of strings. A double-array trie implements this data structure using two integer arrays to represent the tree nodes, providing very efficient lookup, `O(n)` in the length `n` of the input string and no hashing, while it uses little memory and is quick and easy to load and store to disk.

For more information, see https://www.linux.thai.net/~thep/datrie/

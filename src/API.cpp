SmallParsimony(shared_ptr<int> idx_arr, shared_ptr<int> children_arr,
               shared_ptr<char> char_list, int num_char_trees, int num_nodes) {
  /*
   * idx_arr: store starting position of "children_arr".
   * 		(all leaf id is at the very begining; total length is (n+1 (i.e.
   * 1 is root)), with last one is root's index; leaves indices is -1)
   *
   * children_arr: store children node id. (length are always multiple of 2)
   * e.g. idx_arr = [-1, -1, -1,-1, 0, 2, 4]; children_arr=[0,1, 2,3, 4,5]
   *
   * char_list: length is (total number of single-char_trees) X (number of node
   * + 1(root)) e.g. if string length of a leaf if 3 and 2 leaves for each tree,
   *      then char_list=[('A','C'), ('C', 'A'), ('G', 'T')] (i.e. each () is
   * for one single char tree)
   *
   * num_char_trees: total number of single-char_trees, which is also the total
   * length of a leaf's string.
   *
   * num_nodes: n + 1, which is total number of nodes, plus one root node.
   */
}

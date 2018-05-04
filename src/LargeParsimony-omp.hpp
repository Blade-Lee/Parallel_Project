//
//  LargeParsimony.hpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//

#ifndef LargeParsimony_hpp
#define LargeParsimony_hpp

#include <omp.h>
#include <stdio.h>
#include <deque>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#endif /* LargeParsimony_hpp */

using namespace std;

class LargeParsimony {
 public:
  // for global
  int num_threads_;
  int num_char_trees_;
  int num_nodes_;  // not including the root
  int num_leaves_;
  int num_edges_;
  int unrooted_undirectional_tree_len_;
  int rooted_directional_tree_len_;
  int rooted_char_list_len_;
  shared_ptr<int> unrooted_undirectional_tree_;  // n nodes, leaf has 1 edge,
                                                 // other 3, always
                                                 // chage after calling
  // nearest_neighbor_interchage(int a, int b,
  // int b_child)
  shared_ptr<int> unrooted_undirectional_idx_arr_;  // n nodes, never change!!!
  shared_ptr<char> rooted_char_list_;  // (str_len)*(N + 1), never change!!!

  // for final result
  int min_large_parsimony_score_ = int(1e8);
  deque<shared_ptr<int>> unrooted_undirectional_tree_queue_;
  deque<shared_ptr<string>> string_list_queue_;

  // for internal use
  // unrooted_undirectional_tree for
  // internal exchange use
  shared_ptr<int> rooted_directional_tree_;  // (n+1) nodes, parent-children arr
  shared_ptr<int> rooted_directional_idx_arr_;  // (n+1) nodes
  shared_ptr<int>
      edges_;  // for get_edges_from_unrooted_undirectional_tree() use
  shared_ptr<bool>
      visited_;  // for get_edges_from_unrooted_undirectional_tree use
  unique_ptr<char[]> map_char_idx_;
  deque<shared_ptr<int>> tmp_unrooted_undirectional_tree_queue_;
  deque<shared_ptr<string>> tmp_string_list_queue_;

  LargeParsimony(shared_ptr<int> unrooted_undirectional_tree,
                 shared_ptr<int> unrooted_undirectional_idx_arr,
                 shared_ptr<char> rooted_char_list, int num_nodes,
                 int num_leaves, int num_char_trees, int num_threads)
      : num_threads_{num_threads},
        num_char_trees_{num_char_trees},
        num_nodes_{num_nodes},
        num_leaves_{num_leaves},
        num_edges_{num_nodes - num_leaves - 1},
        unrooted_undirectional_tree_len_{(num_nodes - 1) * 2},
        rooted_directional_tree_len_{(num_nodes + 1 - num_leaves) * 2},
        rooted_char_list_len_{(num_nodes + 1) * num_char_trees},
        unrooted_undirectional_tree_{unrooted_undirectional_tree},
        unrooted_undirectional_idx_arr_{unrooted_undirectional_idx_arr},
        rooted_char_list_{rooted_char_list} {
    rooted_directional_tree_ = shared_ptr<int>(
        new int[rooted_directional_tree_len_], [](int* p) { delete[] p; });
    rooted_directional_idx_arr_ =
        shared_ptr<int>(new int[num_nodes + 1], [](int* p) { delete[] p; });
    // internal char->int tool
    map_char_idx_ =
        unique_ptr<char[]>(new char[26]);  // map each char to a index
    map_char_idx_.get()['A' - 'A'] = 0;
    map_char_idx_.get()['C' - 'A'] = 1;
    map_char_idx_.get()['G' - 'A'] = 2;
    map_char_idx_.get()['T' - 'A'] = 3;
    // below for get_edges_from_unrooted_undirectional_tree() use
    edges_ =
        shared_ptr<int>(new int[num_edges_ * 2], [](int* p) { delete[] p; });
    visited_ =
        shared_ptr<bool>(new bool[num_nodes_], [](bool* p) { delete[] p; });
  }

  ~LargeParsimony() = default;
  // Define Small Parsimony Logics
  int run_small_parsimony_string(int num_char_trees, char* rooted_char_list,
                                 int* rooted_directional_tree,
                                 int* rooted_directional_idx_arr,
                                 string* string_list, int num_nodes) {
    // Main logic
    int total_score = 0;
    char ACGT_arr[4] = {'A', 'C', 'G', 'T'};
    for (int i = 0; i < num_char_trees; i++) {
      char* cur_rooted_char_list_idx = rooted_char_list + i * num_nodes;
      int cur_score = run_small_parsimony_char(
          cur_rooted_char_list_idx, rooted_directional_tree,
          rooted_directional_idx_arr, num_nodes);
      // add to final total score
      total_score += cur_score;
      // append char list to current string list
      for (int i = 0; i < num_nodes - 1; i++) {
        string_list[i] += ACGT_arr[int(cur_rooted_char_list_idx[i])];
      }
    }
    return total_score;
  }
  // minimal granularity cell
  int run_small_parsimony_char(char* rooted_char_list,
                               int* rooted_directional_tree,
                               int* rooted_directional_idx_arr,
                               int num_nodes) {  // use current char list and
                                                 // global tree structure to
                                                 // calculate
    // input: char list; directional & rooted tree given as
    // rooted_directional_tree return: the small parsimony score of the char
    // tree and also write the assigned chars to the global rooted_char_list

    // local allocation
    // using unique_ptr inside here, cause unique_ptr has partial definition
    // for
    // unique_ptr
    unique_ptr<int[]> s_v_k(new int[num_nodes * 4]);  // indicate the
                                                      // score of node v
                                                      // choosing k char
    unique_ptr<unsigned char[]> tag(
        new unsigned char[num_nodes]);  // indicate if the noed i is ripe
    unique_ptr<unsigned char[]> back_track_arr(
        new unsigned char[num_nodes * 8]);  // indicate for each node,
                                            // chosen a char of 4, what is
                                            // the best char for its left
                                            // & right children.

    // initialization (no need to initialize back_track_arr)
    int infinity = int(1e8);

    for (int i = 0; i < num_nodes; i++) {
      int bias = 4 * i;
      char leaf_char = rooted_char_list[i];
      int node_idx = rooted_directional_idx_arr[i];  // if -1, then it is a leaf
      for (int j = 0; j < 4; j++) {
        s_v_k.get()[bias + j] =
            infinity *
            int(node_idx == -1 && j != leaf_char);  // if it is a leaves && it
                                                    // is not the leave char,
                                                    // assign it to infinity
      }
      tag.get()[i] = int(node_idx == -1);  // all leaves are ripe already
    }

    int root = -1;  // cur node
    int min_parsimony_score = infinity;
    int root_char_idx = '#';

    // main logic
    while (true) {
      // find a ripe node in the tree
      int ripe_node = -1;
      int daughter = -1;
      int son = -1;
      for (int i = 0; i < num_nodes; i++) {
        if (!tag.get()[i]) {  // if tag(i) is 0
          int bias = rooted_directional_idx_arr[i];
          daughter = rooted_directional_tree[bias];
          son = rooted_directional_tree[bias + 1];
          if (tag.get()[daughter] && tag.get()[son]) {
            ripe_node = i;
            break;
          }
        }
      }

      // if no ripe, root has been found and dealed with
      if (ripe_node == -1) {
        break;
      }
      // find one ripe node
      root = ripe_node;
      min_parsimony_score = infinity;
      root_char_idx = '#';
      tag.get()[root] = 1;
      for (int i = 0; i < 4; i++) {
        char min_left_char_idx = '#';
        char min_right_char_idx = '#';

        int left_min_score = infinity;
        int right_min_score = infinity;

        int offset_left = 4 * daughter;
        int offset_right = 4 * son;

        for (int left_i = 0; left_i < 4; left_i++) {
          int tmp_score = s_v_k.get()[offset_left + left_i] + int(i != left_i);
          if (tmp_score < left_min_score) {
            left_min_score = tmp_score;
            min_left_char_idx = left_i;
          }
        }
        for (int right_i = 0; right_i < 4; right_i++) {
          int tmp_score =
              s_v_k.get()[offset_right + right_i] + int(i != right_i);
          if (tmp_score < right_min_score) {
            right_min_score = tmp_score;
            min_right_char_idx = right_i;
          }
        }
        int cur_total_score = left_min_score + right_min_score;
        s_v_k.get()[root * 4 + i] = cur_total_score;
        if (cur_total_score < min_parsimony_score) {
          min_parsimony_score = cur_total_score;
          root_char_idx = i;
        }
        int back_track_arr_offset = root * 8 + i * 2;
        back_track_arr.get()[back_track_arr_offset] = min_left_char_idx;
        back_track_arr.get()[back_track_arr_offset + 1] = min_right_char_idx;
      }
    }
    // No ripe node any more, root is the fianl root now, calcuate the final
    // score and fill up the char array (tree)
    queue<int> q;
    q.push(root);
    rooted_char_list[root] = root_char_idx;
    while (!q.empty()) {
      int parent = q.front();
      q.pop();
      char min_char_idx = rooted_char_list[parent];
      int child_idx = rooted_directional_idx_arr[parent];
      if (child_idx != -1) {  // if it is not a leaf
        int left_child_id = rooted_directional_tree[child_idx];
        int right_child_id = rooted_directional_tree[child_idx + 1];

        int tmp_idx = parent * 8 + 2 * min_char_idx;
        char left_min_char_idx = back_track_arr.get()[tmp_idx];
        char right_min_char_idx = back_track_arr.get()[tmp_idx + 1];

        rooted_char_list[left_child_id] = left_min_char_idx;
        rooted_char_list[right_child_id] = right_min_char_idx;

        if (rooted_directional_idx_arr[left_child_id] != -1)
          q.push(left_child_id);
        if (rooted_directional_idx_arr[right_child_id] != -1)
          q.push(right_child_id);
      }
    }
    return min_parsimony_score;
  }

  /*
      get a new (unrooted_undirectional_tree,) from old
     (unrooted_undirectional_tree) by interchange of an given edge the return
     tree is deep copies of the original one cause gonna reuse it for all the
     edges in it
   */
  void nearest_neighbor_interchage(int a, int b, int a_child, int b_child,
                                   int* unrooted_undirectional_idx_arr,
                                   int* cur_unrooted_undirectional_tree) {
    // given an edge (node1, node2)
    // b_child is the child of b to exchange with a's left child
    int idx_a = unrooted_undirectional_idx_arr[a];
    int idx_b = unrooted_undirectional_idx_arr[b];
    int idx_a_child = unrooted_undirectional_idx_arr[a_child];
    int idx_b_child = unrooted_undirectional_idx_arr[b_child];
    while (cur_unrooted_undirectional_tree[idx_a] != a_child) {
      idx_a++;
    }
    while (cur_unrooted_undirectional_tree[idx_b] != b_child) {
      idx_b++;
    }
    while (cur_unrooted_undirectional_tree[idx_a_child] != a) {
      idx_a_child++;
    }
    while (cur_unrooted_undirectional_tree[idx_b_child] != b) {
      idx_b_child++;
    }
    cur_unrooted_undirectional_tree[idx_a] = b_child;
    cur_unrooted_undirectional_tree[idx_b] = a_child;
    cur_unrooted_undirectional_tree[idx_a_child] = b;
    cur_unrooted_undirectional_tree[idx_b_child] = a;
  }

  // make the unrooted & undirectional tree rooted & directional
  // call this function every time before small parsimony to generate input
  // for
  // it
  void make_tree_rooted_directional(int* unrooted_undirectional_idx_arr,
                                    int* cur_unrooted_undirectional_tree,
                                    int* rooted_directional_idx_arr,
                                    int* rooted_directional_tree,
                                    int num_nodes) {
    // a deep copy for rooted_char_list (we want to keep a clean original copy
    // of this) unrooted_undirectional_tree to rooted_directional_tree
    // unrooted_undirectional_idx_arr to rooted_directional_idx_arr
    auto tmp_undirected_idx = unrooted_undirectional_idx_arr;
    auto tmp_neighbor_arr = cur_unrooted_undirectional_tree;
    auto tmp_directed_idx = rooted_directional_idx_arr;
    auto tmp_children_arr = rooted_directional_tree;

    int root = num_nodes;
    int left = num_nodes - 1;
    int right = tmp_neighbor_arr[tmp_undirected_idx[left]];
    int next_children = 0;

    for (int i = 0; i < num_nodes + 1; ++i) {
      tmp_directed_idx[i] = -1;
    }

    auto temp_start = next_children;
    tmp_directed_idx[root] = temp_start;
    tmp_children_arr[temp_start++] = left;
    tmp_children_arr[temp_start++] = right;
    next_children = temp_start;

    queue<int> q;
    q.emplace(left);
    q.emplace(right);
    unordered_set<int> visited;
    visited.insert(root);
    visited.insert(left);
    visited.insert(right);

    while (!q.empty()) {
      auto cur_node = q.front();
      q.pop();

      if (cur_node < num_leaves_) {
        continue;
      }

      auto undirected_start_pos = tmp_undirected_idx[cur_node];
      auto directed_start_pos = next_children;
      tmp_directed_idx[cur_node] = directed_start_pos;
      for (int i = undirected_start_pos; i < undirected_start_pos + 3; ++i) {
        auto neighbor = tmp_neighbor_arr[i];
        if (visited.find(neighbor) == visited.end()) {
          tmp_children_arr[directed_start_pos++] = neighbor;
          q.emplace(neighbor);
          visited.insert(neighbor);
        }
      }
      next_children = directed_start_pos;
    }
  }

  // return edges array containing only the internal edges denoted as (a, b)
  // for
  // the internal exchange for unrooted & undirectional tree
  void get_edges_from_unrooted_undirectional_tree(
      int num_leaves, int num_nodes, int* unrooted_undirectional_idx_arr,
      int* unrooted_undirectional_tree, int* edges, bool* visited) {
    for (int i = num_leaves; i < num_nodes; i++) {
      visited[i] = false;
    }
    int edges_ptr = 0;
    for (int i = num_leaves; i < num_nodes; i++) {
      int a = i;
      visited[a] = true;
      int b_idx = unrooted_undirectional_idx_arr[i];
      for (int j = b_idx; j < b_idx + 3; j++) {
        int b = unrooted_undirectional_tree[j];
        if (b >= num_leaves && !visited[b]) {  // edge(a-b) is an internal edge
          edges[edges_ptr++] = a;
          edges[edges_ptr++] = b;
        }
      }
    }
  }

  // creat a deep copy of shared_ptr array and add the ptr to deque
  template <class T>
  void deep_copy_push_back(deque<shared_ptr<T>>& queue, shared_ptr<T> array,
                           int num) {  // must use &
    // first make a deep copy of array
    shared_ptr<T> array_copy =
        shared_ptr<T>(new T[num], [](T* p) { delete[] p; });
    for (int i = 0; i < num; i++) {
      array_copy.get()[i] = array.get()[i];
    }
    queue.push_back(array_copy);
  }
  // creat a shallow copy of shared_ptr array and add the ptr to deque
  template <class T>
  void shallow_copy_push_back(deque<shared_ptr<T>>& queue,
                              shared_ptr<T> array) {  // must use &
    queue.push_back(array);
  }

  // Main entrance function
  void run_large_parsimony() {
    /*
     * input is undirected & unrooted tree; string list
     * each time run a SmallParsimony, we got a directional&rooted array as
     * well
     * as a string list denoted the string for each node. Keep recording the
     * minumum one.
     */
    // #################run small parsimony###########################
    // new a char list to conatain final tree characters assignments
    shared_ptr<int> cur_unrooted_undirectional_tree = shared_ptr<int>(
        new int[unrooted_undirectional_tree_len_], [](int* p) { delete[] p; });
    shared_ptr<int> cur_rooted_directional_idx_arr =
        shared_ptr<int>(new int[num_nodes_ + 1], [](int* p) { delete[] p; });
    shared_ptr<int> cur_rooted_directional_tree = shared_ptr<int>(
        new int[rooted_directional_tree_len_], [](int* p) { delete[] p; });
    shared_ptr<char> cur_rooted_char_list = shared_ptr<char>(
        new char[rooted_char_list_len_], [](char* p) { delete[] p; });
    shared_ptr<string> cur_string_list = shared_ptr<string>(
        new string[num_nodes_], [](string* p) { delete[] p; });
    for (int i = 0; i < unrooted_undirectional_tree_len_; i++) {
      cur_unrooted_undirectional_tree.get()[i] =
          unrooted_undirectional_tree_.get()[i];
    }
    make_tree_rooted_directional(unrooted_undirectional_idx_arr_.get(),
                                 cur_unrooted_undirectional_tree.get(),
                                 cur_rooted_directional_idx_arr.get(),
                                 cur_rooted_directional_tree.get(), num_nodes_);
    for (int i = 0; i < rooted_char_list_len_; i++) {
      char cur_c = rooted_char_list_.get()[i];
      // preprocess rooted_char_list, converti it from char to char(int)
      if (cur_c == 'A' || cur_c == 'C' || cur_c == 'G' || cur_c == 'T') {
        cur_rooted_char_list.get()[i] = map_char_idx_.get()[cur_c - 'A'];
      }
    }
    for (int i = 0; i < num_nodes_; i++) {
      cur_string_list.get()[i] = "";
    }
    // run small parsimony
    int small_parsimony_total_score = run_small_parsimony_string(
        num_char_trees_, cur_rooted_char_list.get(),
        cur_rooted_directional_tree.get(), cur_rooted_directional_idx_arr.get(),
        cur_string_list.get(), num_nodes_ + 1);
    // ################################################################

    // initialization
    int new_score = small_parsimony_total_score;
    deep_copy_push_back<int>(tmp_unrooted_undirectional_tree_queue_,
                             cur_unrooted_undirectional_tree,
                             unrooted_undirectional_tree_len_);
    deep_copy_push_back<string>(tmp_string_list_queue_, cur_string_list,
                                num_nodes_);
    while (!tmp_unrooted_undirectional_tree_queue_.empty()) {
      // record tmp list to final list
      unrooted_undirectional_tree_queue_ =
          tmp_unrooted_undirectional_tree_queue_;
      string_list_queue_ = tmp_string_list_queue_;
      // clear up tmp list
      tmp_unrooted_undirectional_tree_queue_ = deque<shared_ptr<int>>();
      tmp_string_list_queue_ = deque<shared_ptr<string>>();

      min_large_parsimony_score_ =
          new_score--;  // should use new_score -1 is for comparation (here
                        // compatible with weichen's code)

      auto tree_start = unrooted_undirectional_tree_queue_.begin();
      auto tree_end = unrooted_undirectional_tree_queue_.end();
      auto string_i_ptr = string_list_queue_.begin();

      // Allocate global internal array
      int global_arr_len =
          unrooted_undirectional_tree_queue_.size() * num_edges_ * 2;
      shared_ptr<shared_ptr<int>> unrooted_undirectional_tree_global_arr =
          shared_ptr<shared_ptr<int>>(new shared_ptr<int>[global_arr_len],
                                      [](shared_ptr<int>* p) { delete[] p; });
      shared_ptr<shared_ptr<int>> rooted_directional_tree_global_arr =
          shared_ptr<shared_ptr<int>>(new shared_ptr<int>[global_arr_len],
                                      [](shared_ptr<int>* p) { delete[] p; });
      shared_ptr<shared_ptr<int>> rooted_directional_idx_global_arr =
          shared_ptr<shared_ptr<int>>(new shared_ptr<int>[global_arr_len],
                                      [](shared_ptr<int>* p) { delete[] p; });
      shared_ptr<shared_ptr<char>> rooted_char_list_global_arr =
          shared_ptr<shared_ptr<char>>(new shared_ptr<char>[global_arr_len],
                                       [](shared_ptr<char>* p) { delete[] p; });
      // Allocate global output array
      shared_ptr<int> score_global_arr =
          shared_ptr<int>(new int[global_arr_len], [](int* p) { delete[] p; });
      shared_ptr<shared_ptr<string>> string_list_global_arr =
          shared_ptr<shared_ptr<string>>(
              new shared_ptr<string>[global_arr_len],
              [](shared_ptr<string>* p) { delete[] p; });
      for (auto tree_i_ptr = tree_start; tree_i_ptr != tree_end;
           ++tree_i_ptr, ++string_i_ptr) {
        unrooted_undirectional_tree_ = *tree_i_ptr;
        // get all edges for unrooted_undirectional_tree_
        get_edges_from_unrooted_undirectional_tree(
            num_leaves_, num_nodes_, unrooted_undirectional_idx_arr_.get(),
            unrooted_undirectional_tree_.get(), edges_.get(),
            visited_.get());  // write to
                              // edges;
                              // visited
        // For each edge, exchange the internal edges to get 2 new trees
        int length = num_edges_ * 2;
        int i = 0;
        omp_set_num_threads(num_threads_);
#pragma omp parallel for private(i, cur_unrooted_undirectional_tree, \
                                 cur_rooted_directional_idx_arr,     \
                                 cur_rooted_directional_tree,        \
                                 cur_rooted_char_list, cur_string_list)
        for (i = 0; i < length; i += 2) {
          int a = edges_.get()[i];
          int b = edges_.get()[i + 1];
          int a_child_idx = unrooted_undirectional_idx_arr_.get()[a];
          int a_child = unrooted_undirectional_tree_.get()[a_child_idx];
          a_child = a_child == b
                        ? unrooted_undirectional_tree_.get()[a_child_idx + 1]
                        : a_child;
          int b_child_idx = unrooted_undirectional_idx_arr_.get()[b];
          int b_child = -1;
          for (int j = 0; j < 2; j++) {  // exchange b's j_th child in unrooted
                                         // & undirectional tree
            if (j) {
              for (int k = 2; k >= 0; k--) {
                b_child = unrooted_undirectional_tree_.get()[b_child_idx + k];
                if (b_child != a) break;
              }
            } else {
              for (int k = 0; k < 3; k++) {
                b_child = unrooted_undirectional_tree_.get()[b_child_idx + k];
                if (b_child != a) break;
              }
            }

            // ######################Gather all
            // information###################### Duplication spaces
            cur_unrooted_undirectional_tree =
                shared_ptr<int>(new int[unrooted_undirectional_tree_len_],
                                [](int* p) { delete[] p; });
            cur_rooted_directional_idx_arr = shared_ptr<int>(
                new int[num_nodes_ + 1], [](int* p) { delete[] p; });
            cur_rooted_directional_tree =
                shared_ptr<int>(new int[rooted_directional_tree_len_],
                                [](int* p) { delete[] p; });
            cur_rooted_char_list = shared_ptr<char>(
                new char[rooted_char_list_len_], [](char* p) { delete[] p; });
            cur_string_list = shared_ptr<string>(new string[num_nodes_],
                                                 [](string* p) { delete[] p; });

            // must reinitialize below
            for (int i = 0; i < unrooted_undirectional_tree_len_; i++) {
              cur_unrooted_undirectional_tree.get()[i] =
                  unrooted_undirectional_tree_.get()[i];
            }
            nearest_neighbor_interchage(
                a, b, a_child, b_child, unrooted_undirectional_idx_arr_.get(),
                cur_unrooted_undirectional_tree
                    .get());  // writed to cur_unrooted_undirectional_tree
            make_tree_rooted_directional(unrooted_undirectional_idx_arr_.get(),
                                         cur_unrooted_undirectional_tree.get(),
                                         cur_rooted_directional_idx_arr.get(),
                                         cur_rooted_directional_tree.get(),
                                         num_nodes_);
            for (int i = 0; i < rooted_char_list_len_; i++) {
              char cur_c = rooted_char_list_.get()[i];
              // preprocess rooted_char_list, converti it from char to
              // char(int)
              if (cur_c == 'A' || cur_c == 'C' || cur_c == 'G' ||
                  cur_c == 'T') {
                cur_rooted_char_list.get()[i] =
                    map_char_idx_.get()[cur_c - 'A'];
              }
            }
            for (int i = 0; i < num_nodes_; i++) {
              cur_string_list.get()[i] = "";
            }
            // Global assignment
            int global_arr_idx =
                (tree_i_ptr - tree_start) * num_edges_ * 2 + i + j;
            unrooted_undirectional_tree_global_arr.get()[global_arr_idx] =
                cur_unrooted_undirectional_tree;
            rooted_directional_idx_global_arr.get()[global_arr_idx] =
                cur_rooted_directional_idx_arr;
            rooted_directional_tree_global_arr.get()[global_arr_idx] =
                cur_rooted_directional_tree;
            rooted_char_list_global_arr.get()[global_arr_idx] =
                cur_rooted_char_list;
            string_list_global_arr.get()[global_arr_idx] = cur_string_list;
          }
        }
      }
      int i;
      omp_set_num_threads(num_threads_);
#pragma omp parallel for private(i, small_parsimony_total_score)
      for (i = 0; i < global_arr_len; i++) {
        // ##################run smallparsimony############################
        small_parsimony_total_score = run_small_parsimony_string(
            num_char_trees_, rooted_char_list_global_arr.get()[i].get(),
            rooted_directional_tree_global_arr.get()[i].get(),
            rooted_directional_idx_global_arr.get()[i].get(),
            string_list_global_arr.get()[i].get(), num_nodes_ + 1);
        score_global_arr.get()[i] = small_parsimony_total_score;
        // ##################################################################
      }
      // record the minmal one
      for (i = 0; i < global_arr_len; i++) {
        small_parsimony_total_score = score_global_arr.get()[i];
        if (small_parsimony_total_score <= new_score) {  // compare
          if (small_parsimony_total_score < new_score) {
            // first clear tmp list
            tmp_unrooted_undirectional_tree_queue_.clear();
            tmp_string_list_queue_.clear();
            new_score = small_parsimony_total_score;
          }
          // cout << "^^^^^^^in^^^^^^"<<endl;
          // add to tmp list
          shallow_copy_push_back<int>(
              tmp_unrooted_undirectional_tree_queue_,
              unrooted_undirectional_tree_global_arr.get()[i]);
          shallow_copy_push_back<string>(tmp_string_list_queue_,
                                         string_list_global_arr.get()[i]);
        }
      }
    }
  }
};

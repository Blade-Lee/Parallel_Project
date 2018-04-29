//
//  LargeParsimony.hpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//

#ifndef LargeParsimony_hpp
#define LargeParsimony_hpp

#include <stdio.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "SmallParsimony.hpp"
#endif /* LargeParsimony_hpp */
using namespace std;
class LargeParsimony {
 public:
  // for global
  int num_char_trees;
  int num_nodes;  // not including the root
  int num_leaves;
  int num_edges;
  shared_ptr<int>
      unrooted_undirectional_tree;  // n nodes, leaf has 1 edge, other 3, always
                                    // chage after calling
                                    // nearest_neighbor_interchage(int a, int b,
                                    // int b_child)
  shared_ptr<int> unrooted_undirectional_idx_arr;  // n nodes, never change!!!
  shared_ptr<char> rooted_char_list;  // (str_len)*(N + 1), never change!!!

  // for final result
  int min_large_parsimony_score = int(1e8);
  shared_ptr<string> string_list;  // n length; always pointing to the smallest
                                   // structure calculated in SmallParsimony, no
                                   // need to initialize

  // for internal use
  shared_ptr<int>
      cur_unrooted_undirectional_tree;      // must have a copy of
                                            // unrooted_undirectional_tree for
                                            // internal exchange use
  shared_ptr<int> rooted_directional_tree;  // (n+1) nodes, parent-children arr
  shared_ptr<int> rooted_directional_idx_arr;  // (n+1) nodes
  shared_ptr<char> cur_rooted_char_list;       // (n+1) * (str_len) nodes
  shared_ptr<int>
      edges;  // for get_edges_from_unrooted_undirectional_tree() use
  shared_ptr<bool>
      visited;  // for get_edges_from_unrooted_undirectional_tree use

  LargeParsimony(shared_ptr<int> unrooted_undirectional_tree,
                 shared_ptr<int> unrooted_undirectional_idx_arr,
                 shared_ptr<char> rooted_char_list, int num_nodes,
                 int num_leaves, int num_char_trees) {
    cout << "Large Parsimony constructed." << endl;
    this->unrooted_undirectional_tree = unrooted_undirectional_tree;
    int unrooted_undirectional_tree_len = (num_nodes - 1) * 2;
    this->cur_unrooted_undirectional_tree = shared_ptr<int>(
        new int[unrooted_undirectional_tree_len], [](int* p) { delete[] p; });
    for (int i = 0; i < unrooted_undirectional_tree_len; i++) {
      this->cur_unrooted_undirectional_tree.get()[i] =
          this->unrooted_undirectional_tree.get()[i];
    }
    this->unrooted_undirectional_idx_arr = unrooted_undirectional_idx_arr;
    // rooted_char_list is never change so input size is (str_len)*(N + 1),
    // rooted
    this->rooted_char_list = rooted_char_list;
    this->rooted_directional_tree = shared_ptr<int>(
        new int[(num_nodes + 1 - num_leaves) * 2], [](int* p) { delete[] p; });
    this->rooted_directional_idx_arr =
        shared_ptr<int>(new int[num_nodes + 1], [](int* p) { delete[] p; });
    int rooted_char_list_len = (num_nodes + 1) * num_char_trees;
    // need to get the char list copy below
    this->cur_rooted_char_list = shared_ptr<char>(
        new char[rooted_char_list_len], [](char* p) { delete[] p; });
    for (int i = 0; i < rooted_char_list_len; i++) {
      this->cur_rooted_char_list.get()[i] = this->rooted_char_list.get()[i];
    }
    this->num_nodes = num_nodes;
    this->num_leaves = num_leaves;
    this->num_char_trees = num_char_trees;
    this->num_edges = this->num_nodes - this->num_leaves - 1;
    // below for get_edges_from_unrooted_undirectional_tree() use
    int length = this->num_edges * 2;
    this->edges = shared_ptr<int>(new int[length], [](int* p) { delete[] p; });
    this->visited = shared_ptr<bool>(new bool[this->num_nodes],
                                     [](bool* p) { delete[] p; });
  }
  ~LargeParsimony() { cout << "Large Parsimony deconstructed." << endl; }

  /*
      get a new (unrooted_undirectional_tree,) from old
     (unrootedundirectional_tree) by interchange of an given edge the return
     tree is deep copies of the original one cause gonna reuse it for all the
     edges in it
   */
  void nearest_neighbor_interchage(int a, int b, int a_child, int b_child) {
    // given an edge (node1, node2)
    // b_child is the child of b to exchange with a's left child
    int idx_a = this->unrooted_undirectional_idx_arr.get()[a];
    int idx_b = this->unrooted_undirectional_idx_arr.get()[b];
    int idx_a_child = this->unrooted_undirectional_idx_arr.get()[a_child];
    int idx_b_child = this->unrooted_undirectional_idx_arr.get()[b_child];
    while (this->cur_unrooted_undirectional_tree.get()[idx_a] != a_child) {
      idx_a++;
    }
    while (this->cur_unrooted_undirectional_tree.get()[idx_b] != b_child) {
      idx_b++;
    }
    while (this->cur_unrooted_undirectional_tree.get()[idx_a_child] != a) {
      idx_a_child++;
    }
    while (this->cur_unrooted_undirectional_tree.get()[idx_b_child] != b) {
      idx_b_child++;
    }
    this->cur_unrooted_undirectional_tree.get()[idx_a] = b_child;
    this->cur_unrooted_undirectional_tree.get()[idx_b] = a_child;
    this->cur_unrooted_undirectional_tree.get()[idx_a_child] = b;
    this->cur_unrooted_undirectional_tree.get()[idx_b_child] = a;
  }

  // make the unrooted & undirectional tree rooted & directional
  // call this function every time before small parsimony to generate input for
  // it
  void make_tree_rooted_directional() {
    // a deep copy for rooted_char_list (we want to keep a clean original copy
    // of this) unrooted_undirectional_tree to rooted_directional_tree
    // unrooted_undirectional_idx_arr to rooted_directional_idx_arr
    auto tmp_undirected_idx = this->unrooted_undirectional_idx_arr.get();
    auto tmp_neighbor_arr = this->cur_unrooted_undirectional_tree.get();
    auto tmp_directed_idx = this->rooted_directional_idx_arr.get();
    auto tmp_children_arr = this->rooted_directional_tree.get();

    int root = this->num_nodes;
    int left = this->num_nodes - 1;
    int right = tmp_neighbor_arr[tmp_undirected_idx[left]];
    int next_children = 0;

    for (int i = 0; i < this->num_nodes + 1; ++i) {
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
    // cout<<"reach here1111111111111111111111111"<<endl;
    // cout<<"root = "<<root<<endl;
    // cout<<"left = "<<left<<endl;
    // cout<<"right = "<<right<<endl;
    visited.insert(root);
    visited.insert(left);
    visited.insert(right);

    // cout<<"reach hereXXXXXXXXXXXXXXXXXXXXXX!!!"<<endl;
    while (!q.empty()) {
      auto cur_node = q.front();
      q.pop();

      if (cur_node < num_leaves) {
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
    // cout<<"reach here333333333333333333333333333!!!"<<endl;
  }

  // return edges array containing only the internal edges denoted as (a, b) for
  // the internal exchange for unrooted & undirectional tree
  shared_ptr<int> get_edges_from_unrooted_undirectional_tree() {
    for (int i = this->num_leaves; i < this->num_nodes; i++) {
      this->visited.get()[i] = false;
    }
    int edges_ptr = 0;
    for (int i = this->num_leaves; i < this->num_nodes; i++) {
      int a = i;
      this->visited.get()[a] = true;
      int b_idx = this->unrooted_undirectional_idx_arr.get()[i];
      for (int j = b_idx; j < b_idx + 3; j++) {
        int b = this->unrooted_undirectional_tree.get()[j];
        if (b >= this->num_leaves &&
            !this->visited.get()[b]) {  // edge(a-b) is an internal edge
          this->edges.get()[edges_ptr++] = a;
          this->edges.get()[edges_ptr++] = b;
        }
      }
    }
    return this->edges;
  }

  // Main entrance function
  void run_large_parsimony() {
    /*
     * input is undirected & unrooted tree; string list
     * each time run a SmallParsimony, we got a directional&rooted array as well
     * as a string list denoted the string for each node. Keep recording the
     * minumum one.
     */
    // write to this->rooted_directional_tree and
    // this->rooted_directional_idx_arr
    make_tree_rooted_directional();

    shared_ptr<SmallParsimony> small_parsimony = make_shared<SmallParsimony>(
        this->rooted_directional_idx_arr, this->rooted_directional_tree,
        this->cur_rooted_char_list, this->num_char_trees, num_nodes + 1);
    small_parsimony.get()->run_small_parsimony_string();
    int new_score = small_parsimony.get()->total_score;
    shared_ptr<string> new_string_list = small_parsimony.get()->string_list;
    int unrooted_undirectional_tree_len = (this->num_nodes - 1) * 2;
    shared_ptr<int> new_unrooted_directional_tree = shared_ptr<int>(
        new int[unrooted_undirectional_tree_len], [](int* p) { delete[] p; });
    for (int i = 0; i < unrooted_undirectional_tree_len; i++) {
      new_unrooted_directional_tree.get()[i] =
          this->unrooted_undirectional_tree.get()[i];
    }
    while (new_score < this->min_large_parsimony_score) {
      // cout << "new score is: " << new_score << endl;

      this->min_large_parsimony_score = new_score;
      this->string_list = new_string_list;
      shared_ptr<int> tmp_unrooted_undirectional_tree =
          this->unrooted_undirectional_tree;
      this->unrooted_undirectional_tree = new_unrooted_directional_tree;
      new_unrooted_directional_tree = tmp_unrooted_undirectional_tree;
      shared_ptr<int> edges =
          get_edges_from_unrooted_undirectional_tree();  // write to
                                                         // this->edges;
                                                         // this->visited
    cout<<"*****************************************************************"<<this->min_large_parsimony_score<<endl;
    cout<<"index array:"<<endl;
      for(int i = 0;i<this->num_nodes;i++)
              cout<<i<<":"<<this->unrooted_undirectional_idx_arr.get()[i]<<" ";
    cout<<endl;
    cout<<"neighbor array:"<<endl;
      for(int i = 0;i<(this->num_nodes-1)*2;i++)
          cout<<i<<":"<<this->unrooted_undirectional_tree.get()[i]<<" ";
    cout<<"*****************************************************************"<<endl;
      // For each edge, exchange the internal edges to get 2 new trees
      int length = this->num_edges * 2;
      for (int i = 0; i < length; i += 2) {
        int a = edges.get()[i];
        int b = edges.get()[i + 1];
        int a_child_idx = this->unrooted_undirectional_idx_arr.get()[a];
        int a_child = this->unrooted_undirectional_tree.get()[a_child_idx];
        a_child = a_child == b
                      ? this->unrooted_undirectional_tree.get()[a_child_idx + 1]
                      : a_child;
        int b_child_idx = this->unrooted_undirectional_idx_arr.get()[b];
        int b_child = -1;
        for (int j = 0; j < 2;
             j++) {  // exchange b's j_th child in unrooted & undirectional tree
          if (j) {
            for (int k = 2; k >= 0; k--) {
              b_child =
                  this->unrooted_undirectional_tree.get()[b_child_idx + k];
              if (b_child != a) break;
            }
          } else {
            for (int k = 0; k < 3; k++) {
              b_child =
                  this->unrooted_undirectional_tree.get()[b_child_idx + k];
              if (b_child != a) break;
            }
          }
          // must reinitialize below
          for (int i = 0; i < unrooted_undirectional_tree_len; i++) {
            this->cur_unrooted_undirectional_tree.get()[i] =
                this->unrooted_undirectional_tree.get()[i];
          }
          // begin interchange
          nearest_neighbor_interchage(
              a, b, a_child,
              b_child);  // writed to this->cur_unrooted_undirectional_tree

          // cout<<a<<" "<<b<<" "<<a_child<<" "<<b_child<<endl;
          // for(int i = 0;i<this->num_nodes;i++)
          //     cout<<i<<":"<<this->unrooted_undirectional_idx_arr.get()[i]<<"
          //     ";
          // cout<<endl;
          // for(int i = 0;i<(this->num_nodes-1)*2;i++)
          //     cout<<i<<":"<<this->unrooted_undirectional_tree.get()[i]<<" ";
          // cout<<endl;
          // cout<<"reach here before"<<endl;

          make_tree_rooted_directional();  // write to
                                           // this->cur_unrooted_undirectional_tree;
                                           // this->rooted_directional_idx_arr;
                                           // this->rooted_directional_tree;

          // cout<<"reach here after"<<endl;

          // need to get the char list copy below
          int rooted_char_list_len =
              (this->num_nodes + 1) * this->num_char_trees;
          for (int i = 0; i < rooted_char_list_len; i++) {
            this->cur_rooted_char_list.get()[i] =
                this->rooted_char_list.get()[i];
          }
          // run small parsimony
          small_parsimony = make_shared<SmallParsimony>(
              this->rooted_directional_idx_arr, this->rooted_directional_tree,
              this->cur_rooted_char_list, this->num_char_trees,
              this->num_nodes + 1);
          small_parsimony.get()->run_small_parsimony_string();
          cout<<"*****"<<small_parsimony.get()->total_score<<endl;
          // cout<<"new_score is: "<<new_score<<" small_parsimony->total_score
          // is: "<<small_parsimony.get()->total_score<<endl;
          if (small_parsimony.get()->total_score < new_score) {  // compare
            new_score = small_parsimony.get()->total_score;
            new_string_list = small_parsimony.get()->string_list;
            tmp_unrooted_undirectional_tree = new_unrooted_directional_tree;
            new_unrooted_directional_tree =
                this->cur_unrooted_undirectional_tree;
            this->cur_unrooted_undirectional_tree =
                tmp_unrooted_undirectional_tree;

            // cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
            // for(int i = 0;i<this->num_nodes;i++)
            //     cout<<i<<":"<<this->unrooted_undirectional_idx_arr.get()[i]<<"
            //     ";
            // cout<<endl;
            // for(int i = 0;i<(this->num_nodes-1)*2;i++)
            //     cout<<i<<":"<<new_unrooted_directional_tree.get()[i]<<" ";
            // cout<<endl;
            // cout<<"vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<<endl;
          }
        }
      }
    }
  }
};

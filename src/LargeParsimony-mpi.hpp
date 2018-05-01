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
#include <deque>
#include <queue>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "LargeParsimonyUtil.hpp"
#endif /* LargeParsimony_hpp */
using namespace std;

class LargeParsimony {
 public:
  // for global
  int num_char_trees;
  int num_nodes;  // not including the root
  int num_leaves;
  int num_edges;
  int unrooted_undirectional_tree_len;
  shared_ptr<int>
      unrooted_undirectional_tree;  // n nodes, leaf has 1 edge, other 3, always
                                    // chage after calling
                                    // nearest_neighbor_interchage(int a, int b,
                                    // int b_child)
  shared_ptr<int> unrooted_undirectional_idx_arr;  // n nodes, never change!!!
  shared_ptr<char> rooted_char_list;  // (str_len)*(N + 1), never change!!!

  // for final result
  int min_large_parsimony_score = int(1e8);
  deque<shared_ptr<int>> unrooted_undirectional_tree_queue;
  deque<shared_ptr<string>> string_list_queue;

  // for internal use
  shared_ptr<int> cur_unrooted_undirectional_tree;  // must have a copy of
  // unrooted_undirectional_tree for
  // internal exchange use
  shared_ptr<int> rooted_directional_tree;  // (n+1) nodes, parent-children arr
  shared_ptr<int> rooted_directional_idx_arr;  // (n+1) nodes
  shared_ptr<char> cur_rooted_char_list;       // (n+1) * (str_len) nodes
  shared_ptr<int>
      edges;  // for get_edges_from_unrooted_undirectional_tree() use
  shared_ptr<bool>
      visited;  // for get_edges_from_unrooted_undirectional_tree use
  unique_ptr<char[]> map_char_idx;
  deque<shared_ptr<int>> tmp_unrooted_undirectional_tree_queue;
  deque<shared_ptr<string>> tmp_string_list_queue;

  LargeParsimony(shared_ptr<int> unrooted_undirectional_tree,
                 shared_ptr<int> unrooted_undirectional_idx_arr,
                 shared_ptr<char> rooted_char_list, int num_nodes,
                 int num_leaves, int num_char_trees) {
    cout << "Large Parsimony constructed." << endl;

    this->num_nodes = num_nodes;
    this->num_leaves = num_leaves;
    this->num_char_trees = num_char_trees;
    this->num_edges = num_nodes - num_leaves - 1;
    this->unrooted_undirectional_tree_len = (num_nodes - 1) * 2;
    this->unrooted_undirectional_tree = unrooted_undirectional_tree;
    this->cur_unrooted_undirectional_tree =
        shared_ptr<int>(new int[this->unrooted_undirectional_tree_len],
                        [](int* p) { delete[] p; });
    for (int i = 0; i < this->unrooted_undirectional_tree_len; i++) {
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
    // below for get_edges_from_unrooted_undirectional_tree() use

    this->edges = shared_ptr<int>(new int[this->num_edges * 2],
                                  [](int* p) { delete[] p; });
    this->visited = shared_ptr<bool>(new bool[this->num_nodes],
                                     [](bool* p) { delete[] p; });
    this->map_char_idx = unique_ptr<char[]>(new char[26]);  // map each char to a index
    this->map_char_idx.get()['A' - 'A'] = 0;
    this->map_char_idx.get()['C' - 'A'] = 1;
    this->map_char_idx.get()['G' - 'A'] = 2;
    this->map_char_idx.get()['T' - 'A'] = 3;
  }
  
  ~LargeParsimony() { cout << "Large Parsimony deconstructed." << endl; }

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
    
    // #################run small parsimony###########################
    // new a string list to conatain final tree characters assignments
    shared_ptr<string> string_list =
        shared_ptr<string>(new string[this->num_nodes], [](string* p) {
          delete[] p;
        });  // N, assign each of node a string finally
    for (int i = 0; i < this->num_nodes; i++) {
      string_list.get()[i] = "";
    }
    // run small parsimony
    int small_parsimony_total_score = run_small_parsimony_string(
        this->num_char_trees, this->cur_rooted_char_list.get(),
        this->rooted_directional_tree.get(),
        this->rooted_directional_idx_arr.get(), string_list.get(),
        this->num_nodes + 1);
    // ################################################################

    // initialization
    int new_score = small_parsimony_total_score;

    // initialize deque. Noted that (new_score/new_string_list) are always the
    // minimal (score/string_list) in the
    // this->tmp_unrooted_undirectional_tree_queue
    deep_copy_push_back<int>(this->tmp_unrooted_undirectional_tree_queue,
                             this->unrooted_undirectional_tree,
                             this->unrooted_undirectional_tree_len);
    deep_copy_push_back<string>(this->tmp_string_list_queue,
                                string_list,
                                this->num_nodes);
    while (!this->tmp_unrooted_undirectional_tree_queue.empty()) {
      // cout<<"----------------size of tmp list--------"<<
      // tmp_unrooted_undirectional_tree_queue.size()<<endl;

      // record tmp list to final list
      this->unrooted_undirectional_tree_queue =
          this->tmp_unrooted_undirectional_tree_queue;
      this->string_list_queue = this->tmp_string_list_queue;
      // clear up tmp list
      this->tmp_unrooted_undirectional_tree_queue = deque<shared_ptr<int>>();
      this->tmp_string_list_queue = deque<shared_ptr<string>>();

      this->min_large_parsimony_score =
          new_score--;  // should use new_score -1 is for comparation (here
                        // compatible with weichen's code)

      auto tree_i_ptr = this->unrooted_undirectional_tree_queue.begin();
      auto tree_end = this->unrooted_undirectional_tree_queue.end();
      auto string_i_ptr = this->string_list_queue.begin();

      for (; tree_i_ptr != tree_end; ++tree_i_ptr, ++string_i_ptr) {
        this->unrooted_undirectional_tree = *tree_i_ptr;
        // get all edges for this->unrooted_undirectional_tree
        this->edges = get_edges_from_unrooted_undirectional_tree(
            this->num_leaves, this->num_nodes, this->unrooted_undirectional_idx_arr,
            this->unrooted_undirectional_tree, this->edges,
            this->visited);  // write to
                             // this->edges;
                             // this->visited
        // For each edge, exchange the internal edges to get 2 new trees
        int length = this->num_edges * 2;
        for (int i = 0; i < length; i += 2) {
          int a = this->edges.get()[i];
          int b = this->edges.get()[i + 1];
          int a_child_idx = this->unrooted_undirectional_idx_arr.get()[a];
          int a_child = this->unrooted_undirectional_tree.get()[a_child_idx];
          a_child =
              a_child == b
                  ? this->unrooted_undirectional_tree.get()[a_child_idx + 1]
                  : a_child;
          int b_child_idx = this->unrooted_undirectional_idx_arr.get()[b];
          int b_child = -1;
          for (int j = 0; j < 2; j++) {  // exchange b's j_th child in unrooted
                                         // & undirectional tree
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
            for (int i = 0; i < this->unrooted_undirectional_tree_len; i++) {
              this->cur_unrooted_undirectional_tree.get()[i] =
                  this->unrooted_undirectional_tree.get()[i];
            }
            // begin interchange
            nearest_neighbor_interchage(
                a, b, a_child, b_child, this->unrooted_undirectional_idx_arr,
                this->cur_unrooted_undirectional_tree);  // writed to
                                                         // this->cur_unrooted_undirectional_tree
            make_tree_rooted_directional(this->unrooted_undirectional_idx_arr,
                                         this->cur_unrooted_undirectional_tree,
                                         this->rooted_directional_idx_arr,
                                         this->rooted_directional_tree,
                                         this->num_nodes);  // write to
            // this->rooted_directional_idx_arr;
            // this->rooted_directional_tree;
            // need to get the char list copy below
            int rooted_char_list_len =
                (this->num_nodes + 1) * this->num_char_trees;
            for (int i = 0; i < rooted_char_list_len; i++) {
              char cur_c = this->rooted_char_list.get()[i];
              // preprocess rooted_char_list, converti it from char to char(int)
              if (cur_c == 'A' || cur_c == 'C' || cur_c == 'G' || cur_c == 'T') {
                this->cur_rooted_char_list.get()[i] = this->map_char_idx.get()[cur_c - 'A'];
              }
            }
            // #################run small parsimony###########################
            // new a string list to conatain final tree characters assignments
            string_list =
                shared_ptr<string>(new string[this->num_nodes], [](string* p) {
                  delete[] p;
                });  // N, assign each of node a string finally
            for (int i = 0; i < this->num_nodes; i++) {
              string_list.get()[i] = "";
            }
            // Main logic
            small_parsimony_total_score = run_small_parsimony_string(
                this->num_char_trees, this->cur_rooted_char_list.get(),
                this->rooted_directional_tree.get(),
                this->rooted_directional_idx_arr.get(), string_list.get(),
                this->num_nodes + 1);
            // ################################################################

            // record the minmal one
            if (small_parsimony_total_score <= new_score) {  // compare
              if (small_parsimony_total_score < new_score) {
                // first clear tmp list
                this->tmp_unrooted_undirectional_tree_queue.clear();
                this->tmp_string_list_queue.clear();
                new_score = small_parsimony_total_score;
              }
              // cout << "^^^^^^^in^^^^^^"<<endl;
              // add to tmp list
              deep_copy_push_back<int>(
                  this->tmp_unrooted_undirectional_tree_queue,
                  cur_unrooted_undirectional_tree,
                  this->unrooted_undirectional_tree_len);
              deep_copy_push_back<string>(this->tmp_string_list_queue,
                                          string_list, this->num_nodes);
            }
          }
        }
      }
    }
  }
};

//
//  SmallParsimony.hpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//

#ifndef SmallParsimony_hpp
#define SmallParsimony_hpp

#include <stdio.h>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#endif /* SmallParsimony_hpp */

using namespace std;

class SmallParsimony {
  // trees here are all rooted and directed
 public:
  // char set for each tree. All str_len sets.
  shared_ptr<char> char_list;  // (str_len)*(N+1)

  // Tree structure
  shared_ptr<int> idx_arr;       // N+1, last one is root
  shared_ptr<int> children_arr;  // #internal nodes * 2

  int num_char_trees;
  int num_nodes;

  // final results
  int total_score = -1;            // final result
  shared_ptr<string> string_list;  // N, assign each of node a string finally
                                   // (problem of cirle reference???)

  SmallParsimony(shared_ptr<int> idx_arr, shared_ptr<int> children_arr,
                 shared_ptr<char> char_list, int num_char_trees,
                 int num_nodes) {
    // cout << "Small Parsimony constructed." << endl;
    this->char_list = char_list;
    this->idx_arr = idx_arr;
    this->children_arr = children_arr;

    this->num_char_trees = num_char_trees;
    this->num_nodes = num_nodes;  // Noted that this number has included root
    this->string_list = shared_ptr<string>(new string[this->num_nodes - 1],
                                           [](string* p) { delete[] p; });
    // init all to "" for convenience of concatenating chars
    string* string_list_ptr = this->string_list.get();

    unique_ptr<char[]> map_char_idx(
        new char[26]());  // map each char to a index
    map_char_idx.get()['A' - 'A'] = 0;
    map_char_idx.get()['C' - 'A'] = 1;
    map_char_idx.get()['G' - 'A'] = 2;
    map_char_idx.get()['T' - 'A'] = 3;
    for (int i = 0; i < this->num_nodes - 1; i++) {
      string_list_ptr[i] = "";
    }

    int char_list_len = this->num_char_trees * this->num_nodes;
    for (int i = 0; i < char_list_len; i++) {
      char cur_c = this->char_list.get()[i];
      if (cur_c == 'A' || cur_c == 'C' || cur_c == 'G' || cur_c == 'T') {
        this->char_list.get()[i] = map_char_idx.get()[cur_c - 'A'];
      }
    }
  }
  ~SmallParsimony() {
    // cout << "Small Parsimony decostructed." << endl;
  }
  void run_small_parsimony_string() {
    this->total_score = 0;

    // cout<<"char list: "<<endl;
    // for(int i = 0; i < this->num_char_trees * this->num_nodes; i++)
    //     cout<<int(this->char_list.get()[i])<<" ";
    // cout<<endl;

    char ACGT_arr[4] = {'A', 'C', 'G', 'T'};

    for (int i = 0; i < this->num_char_trees; i++) {
      char* cur_char_list_idx = this->char_list.get() + i * this->num_nodes;
      int cur_score = run_small_parsimony_char(cur_char_list_idx);

      // add to final total score
      this->total_score += cur_score;

      // append char list to current string list
      for (int i = 0; i < this->num_nodes - 1; i++) {
        this->string_list.get()[i] += ACGT_arr[int(cur_char_list_idx[i])];
      }
    }

    // for(int i = 0; i < this->num_nodes - 1; i++){
    //     cout<<this->string_list.get()[i]<<" ";
    // }
  }

  int run_small_parsimony_char(char* char_list) {  // use current char list and
                                                   // global tree structure to
                                                   // calculate
    // input: char list; directional & rooted tree given as children_arr
    // return: the small parsimony score of the char tree and also write the
    // assigned chars to the global char_list

    // local allocation
    // using unique_ptr inside here, cause unique_ptr has partial definition for
    // unique_ptr
    unique_ptr<int[]> s_v_k(new int[this->num_nodes * 4]);  // indicate the
                                                            // score of node v
                                                            // choosing k char
    unique_ptr<unsigned char[]> tag(
        new unsigned char[this->num_nodes]);  // indicate if the noed i is ripe
    unique_ptr<unsigned char[]> back_track_arr(
        new unsigned char[this->num_nodes * 8]);  // indicate for each node,
                                                  // chosen a char of 4, what is
                                                  // the best char for its left
                                                  // & right children.

    // initialization (no need to initialize back_track_arr)
    int infinity = int(1e8);

    for (int i = 0; i < this->num_nodes; i++) {
      int bias = 4 * i;
      char leaf_char = char_list[i];
      int node_idx = this->idx_arr.get()[i];  // if -1, then it is a leaf
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
      for (int i = 0; i < this->num_nodes; i++) {
        if (!tag.get()[i]) {  // if tag(i) is 0
          int bias = this->idx_arr.get()[i];
          daughter = this->children_arr.get()[bias];
          son = this->children_arr.get()[bias + 1];
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
      // cout<<"############################################################################################"<<endl;
      // cout<<"root is: "<<root<<"  root's char is:
      // "<<int(char_list[root])<<endl; begin DP
      for (int i = 0; i < 4; i++) {
        // cout<<"root's char--"<<i<<"---score is: "<< int(s_v_k.get()[4*root +
        // i])<<"  daughter is: "<< daughter<<"  son is: "<< son <<endl;
        char min_left_char_idx = '#';
        char min_right_char_idx = '#';

        int left_min_score = infinity;
        int right_min_score = infinity;

        int offset_left = 4 * daughter;
        int offset_right = 4 * son;

        for (int left_i = 0; left_i < 4; left_i++) {
          int tmp_score = s_v_k.get()[offset_left + left_i] + int(i != left_i);
          // cout<<"daughter--"<<left_i<<"--'s score is:
          // "<<s_v_k.get()[offset_left + left_i]<<endl;
          if (tmp_score < left_min_score) {
            left_min_score = tmp_score;
            min_left_char_idx = left_i;
          }
        }
        // cout<<"left_min_score: "<<left_min_score<<endl;
        for (int right_i = 0; right_i < 4; right_i++) {
          int tmp_score =
              s_v_k.get()[offset_right + right_i] + int(i != right_i);
          // cout<<"son--"<<right_i<<"--'s score is: "<<s_v_k.get()[offset_right
          // + right_i]<<endl;
          if (tmp_score < right_min_score) {
            right_min_score = tmp_score;
            min_right_char_idx = right_i;
          }
        }
        // cout<<"right_min_score: "<<right_min_score<<endl;
        int cur_total_score = left_min_score + right_min_score;
        s_v_k.get()[root * 4 + i] = cur_total_score;

        // cout<<"$$$$$$$$$$$$$s_v_k---"<<root<<"---i---"<<i<<" =
        // "<<cur_total_score;

        // cout<<"min score is: "<< min_parsimony_score<< " cur_score is: "<<
        // cur_total_score<<endl;
        if (cur_total_score < min_parsimony_score) {
          min_parsimony_score = cur_total_score;
          root_char_idx = i;
        }
        int back_track_arr_offset = root * 8 + i * 2;
        back_track_arr.get()[back_track_arr_offset] = min_left_char_idx;
        back_track_arr.get()[back_track_arr_offset + 1] = min_right_char_idx;

        // cout<<"------------------------------------------------------------------------------------"<<endl;
      }
    }
    // No ripe node any more, root is the fianl root now, calcuate the final
    // score and fill up the char array (tree)
    queue<int> q;
    q.push(root);
    char_list[root] = root_char_idx;
    while (!q.empty()) {
      int parent = q.front();
      q.pop();
      char min_char_idx = char_list[parent];
      int child_idx = this->idx_arr.get()[parent];
      if (child_idx != -1) {  // if it is not a leaf
        int left_child_id = this->children_arr.get()[child_idx];
        int right_child_id = this->children_arr.get()[child_idx + 1];

        int tmp_idx = parent * 8 + 2 * min_char_idx;
        char left_min_char_idx = back_track_arr.get()[tmp_idx];
        char right_min_char_idx = back_track_arr.get()[tmp_idx + 1];

        char_list[left_child_id] = left_min_char_idx;
        char_list[right_child_id] = right_min_char_idx;

        if (this->idx_arr.get()[left_child_id] != -1) q.push(left_child_id);
        if (this->idx_arr.get()[right_child_id] != -1) q.push(right_child_id);
      }
    }
    return min_parsimony_score;
  }
};

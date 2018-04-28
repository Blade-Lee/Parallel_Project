//
//  main.cpp
//  LargeParsimonyProblem
//
//  Created by ShanLi on 2018/4/16.
//  Copyright © 2018 WhistleStop. All rights reserved.
//
//
#include <iostream>
#include "LargeParsimony.hpp"
#include "util.h"

void runBaseline(string file_name, string outfile_name) {
  auto lines = readLines(file_name);

  int num_leaves = stoi(lines.front());
  int cur_leave = num_leaves - 1;
  lines.pop();

  unordered_map<string, int> assign;
  unordered_map<int, unordered_set<int>> neighbors;
  int max_node_idx = -1;

  while (!lines.empty()) {
    auto line = lines.front();
    lines.pop();

    auto pair = getNeighborPair(line, assign, cur_leave);
    int first = get<0>(pair);
    int second = get<1>(pair);
    max_node_idx = first > max_node_idx ? first : max_node_idx;
    max_node_idx = second > max_node_idx ? second : max_node_idx;

    connectNeighborPair(neighbors, first, second);
  }

  int num_char_trees = (assign.begin()->first).length();

  // Convert from Neighbor Map to Undirected Tree
  int num_undirected_nodes = max_node_idx + 1;
  int num_undirected_edges = num_undirected_nodes - 1;

  auto undirected_idx = shared_ptr<int>(new int[num_undirected_nodes],
                                        [](int* p) { delete[] p; });
  auto neighbor_arr = shared_ptr<int>(new int[num_undirected_edges * 2],
                                      [](int* p) { delete[] p; });

  convertNeighborsToUndirectedArr(neighbors, undirected_idx, neighbor_arr);

  // for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
  //   cout << it->first << endl;
  //   for (auto sit = (it->second).begin(); sit != (it->second).end(); ++sit) {
  //     cout << "--" << *sit << endl;
  //   }
  // }

  // cout << "------------------" << endl;

  // auto temp_idx = undirected_idx.get();
  // auto temp_arr = neighbor_arr.get();
  // for (int i = 0; i < num_undirected_nodes; ++i) {
  //   cout << i << endl;
  //   int j_max = i < num_leaves ? 1 : 3;
  //   for (int j = temp_idx[i]; j < temp_idx[i] + j_max; ++j) {
  //     cout << "--" << temp_arr[j] << endl;
  //   }
  // }

  // Directed Tree
  int num_directed_nodes = max_node_idx + 2;
  int num_directed_internal_nodes = num_directed_nodes - num_leaves;

  auto directed_idx =
      shared_ptr<int>(new int[num_directed_nodes], [](int* p) { delete[] p; });

  auto children_arr = shared_ptr<int>(new int[num_directed_internal_nodes * 2],
                                      [](int* p) { delete[] p; });

  auto char_list =
      shared_ptr<char>(new char[num_directed_nodes * num_char_trees],
                       [](char* p) { delete[] p; });

  covertUndirectedToDirected(num_undirected_nodes, num_leaves, undirected_idx,
                             neighbor_arr, directed_idx, children_arr);

  // cout << "------------------" << endl;

  // temp_idx = directed_idx.get();
  // temp_arr = children_arr.get();
  // for (int i = 0; i < num_directed_nodes; ++i) {
  //   cout << i << endl;
  //   if (i < num_leaves) {
  //     cout << "--" << endl;
  //     continue;
  //   }
  //   for (int j = temp_idx[i]; j < temp_idx[i] + 2; ++j) {
  //     cout << "--" << temp_arr[j] << endl;
  //   }
  // }

  // cout << "------------------------" << endl;

  // for (int i = 0; i < num_directed_nodes; ++i) {
  //   cout << (int)temp_idx[i] << " ";
  // }
  // cout << endl;

  initializeCharList(char_list, assign, num_char_trees, num_directed_nodes);

  // auto tmp_char_list = char_list.get();
  // for (int i = 0; i < num_leaves; ++i) {
  //   cout << i << endl << "--";
  //   for (int tree = 0; tree < num_char_trees; ++tree) {
  //     cout << tmp_char_list[tree * num_directed_nodes + i];
  //   }
  //   cout << endl;
  // }
  // cout<<"number of undirected nodes: "<< num_undirected_nodes<<" number of
  // directed nodes: "<< num_directed_nodes<<" number of char trees: "<<
  // num_char_trees<<endl;
  //   for(int i = 0; i < num_directed_nodes; i++){
  //       cout<<directed_idx.get()[i]<<" ";
  //   }
  //   cout<<endl;
  //   for(int i = 0; i < num_directed_internal_nodes * 2; i++){
  //       cout<<children_arr.get()[i]<<" ";
  //   }
  //   cout<<endl;
  //   for(int i = 0; i < num_directed_nodes * num_char_trees; i++){
  //       cout<<char_list.get()[i]<<" ";
  //   }
  //   cout<<endl;
  // run small parsimony
  //  shared_ptr<SmallParsimony> small_parsimony =
  //  make_shared<SmallParsimony>(directed_idx, children_arr, char_list,
  //  num_char_trees, num_directed_nodes);
  // small_parsimony->run_small_parsimony_string();
  //  cout<<"Small Parsimony total score
  //  is:"<<small_parsimony->total_score<<endl;
  // run large parsimony
  shared_ptr<LargeParsimony> large_parsimony = make_shared<LargeParsimony>(
      neighbor_arr, undirected_idx, char_list, num_undirected_nodes, num_leaves,
      num_char_trees);
  large_parsimony.get()->run_large_parsimony();
  cout << "===================================================================="
          "====="
       << endl;
  cout << "the minimum large parsimony score is: "
       << large_parsimony.get()->min_large_parsimony_score << endl;
  for (int i = 0; i < num_undirected_nodes; i++) {
    cout << "node: " << i << " ---> "
         << large_parsimony.get()->string_list.get()[i] << endl;
  }
}

int main(int argc, const char* argv[]) {
  runBaseline("data/dataset_38507_8.txt", "output/cpp_result.txt");
}

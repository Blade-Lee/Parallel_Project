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
    
    int num_char_trees = int((assign.begin()->first).length());
    
    // Convert from Neighbor Map to Undirected Tree
    int num_undirected_nodes = max_node_idx + 1;
    int num_undirected_edges = num_undirected_nodes - 1;
    
    auto undirected_idx = shared_ptr<int>(new int[num_undirected_nodes],
                                          [](int* p) { delete[] p; });
    auto neighbor_arr = shared_ptr<int>(new int[num_undirected_edges * 2],
                                        [](int* p) { delete[] p; });
    
    convertNeighborsToUndirectedArr(neighbors, undirected_idx, neighbor_arr);
    
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
    initializeCharList(char_list, assign, num_char_trees, num_directed_nodes);
    cout<<"number of undirected nodes: "<< num_undirected_nodes<<" number of directed nodes: "<< num_directed_nodes<<" number of char trees: "<< num_char_trees<<endl;
    for(int i = 0; i < num_directed_nodes; i++){
        cout<<directed_idx.get()[i]<<" ";
    }
    cout<<endl;
    for(int i = 0; i < num_directed_internal_nodes * 2; i++){
        cout<<children_arr.get()[i]<<" ";
    }
    cout<<endl;
    for(int i = 0; i < num_directed_nodes * num_char_trees; i++){
        cout<<char_list.get()[i]<<" ";
    }
    cout<<endl;
    // run small parsimony
    shared_ptr<SmallParsimony> small_parsimony = make_shared<SmallParsimony>(directed_idx, children_arr, char_list, num_char_trees, num_directed_nodes);
//    small_parsimony->run_small_parsimony_string();
    cout<<"Small Parsimony total score is:"<<small_parsimony->total_score<<endl;
}

int main(int argc, const char * argv[]) {
    runBaseline("data/dataset_38507_8.txt", "output/cpp_result.txt");
}

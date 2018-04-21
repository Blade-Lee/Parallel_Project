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
class LargeParsimony{
public:
    // for global
    int num_char_trees;
    int num_nodes; // not including the root
    int num_leaves;
    int num_edges;
    shared_ptr<int> unrooted_undirectional_tree; // n nodes, leaf has 1 edge, other 3, always chage after calling nearest_neighbor_interchage(int a, int b, int b_child)
    shared_ptr<int> unrooted_undirectional_idx_arr; // n nodes, never change!!!
    shared_ptr<char> rooted_char_list; // (str_len)*(N + 1), never change!!!
    
    // for final result
    int min_large_parsimony_score = int(1e8);
    shared_ptr<string> string_list; // n length; always pointing to the smallest structure calculated in SmallParsimony, no need to initialize
    
    // for internal use
    shared_ptr<int> rooted_directional_tree; // (n+1) nodes, parent-children arr
    shared_ptr<char> cur_rooted_char_list;// (n+1) * (str_len) nodes,
    shared_ptr<int> rooted_directional_idx_arr;// (n+1) nodes
    
    LargeParsimony(shared_ptr<int> unrooted_undirectional_tree, shared_ptr<int>unrooted_undirectional_idx_arr, shared_ptr<char> rooted_char_list, int num_nodes, int num_leaves, int num_char_trees){
        this->unrooted_undirectional_tree = unrooted_undirectional_tree;
        this->unrooted_undirectional_idx_arr = unrooted_undirectional_idx_arr;
        // rooted_char_list is never change so input size is (str_len)*(N + 1), rooted
        this->rooted_char_list = rooted_char_list;
        this->num_nodes = num_nodes;
        this->num_leaves = num_leaves;
        this->num_char_trees = num_char_trees;
        this->num_edges = this->num_nodes - this->num_leaves - 1;
    }
    
    /* 
        get a new (unrooted_undirectional_tree, unrooted_undirectional_idx_arr) from old (unrooted_undirectional_tree, unrooted_undirectional_idx_arr) by interchange of an given edge
        the return tree is deep copies of the original one cause gonna reuse it for all the edges in it
     */
    shared_ptr<int> nearest_neighbor_interchage(int a, int b, int b_child){
        // given an edge (node1, node2)
        // b_child is the child of b to exchange with a's left child
        
        return NULL;
    }
    
    // make the unrooted & undirectional tree rooted & directional
    // call this function every time before small parsimony to generate input for it
    void make_tree_rooted_directional() {
        // a deep copy for rooted_char_list (we want to keep a clean original copy of this)
        // unrooted_undirectional_tree to rooted_directional_tree
        // unrooted_undirectional_idx_arr to rooted_directional_idx_arr
        auto tmp_undirected_idx = this->unrooted_undirectional_idx_arr.get();
        auto tmp_neighbor_arr = this->unrooted_undirectional_tree.get();
        auto tmp_directed_idx = this->rooted_directional_idx_arr.get();
        auto tmp_children_arr = this->unrooted_undirectional_tree.get();
        
        int root = this->num_nodes;
        int left = this->num_nodes - 1;
        int right = tmp_neighbor_arr[tmp_neighbor_arr[left]];
        int next_children = 0;
        
        for (int i = 0; i < (this->num_nodes + 1 - this->num_leaves) * 2; ++i) {
            tmp_children_arr[i] = -1;
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
            
            if (cur_node < this->num_leaves) {
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
    
    // return edges array containing only the internal edges denoted as (a, b) for the internal exchange for unrooted & undirectional tree
    shared_ptr<int> get_edges_from_unrooted_undirectional_tree(){
        int length = this->num_edges * 2;
        shared_ptr<int> edges (new int[length], [](int* p){delete[] p;});
        int edges_ptr = 0;
        for(int i = this->num_leaves; i < this->num_nodes; i++){
            int a = i;
            int b_idx = this->unrooted_undirectional_idx_arr.get()[i];
            for(int j = b_idx; j < b_idx + 3; j++){
                int b = this->unrooted_undirectional_tree.get()[j];
                if(b >= this->num_leaves){ // edge(a-b) is an internal edge
                    edges.get()[edges_ptr++] = a;
                    edges.get()[edges_ptr++] = b;
                }
            }
        }
        return edges;
    }
    
    // Main entrance function
    void run_large_parsimony(){
        /*
         * input is undirected & unrooted tree; string list
         * each time run a SmallParsimony, we got a directional&rooted array as well as
         * a string list denoted the string for each node. Keep recording the minumum one.
         */
        // write to this->rooted_directional_tree and this->rooted_directional_idx_arr
        make_tree_rooted_directional();
        shared_ptr<SmallParsimony> small_parsimony = make_shared<SmallParsimony>(this->rooted_directional_idx_arr, this->rooted_directional_tree, this->cur_rooted_char_list, this->num_char_trees, num_nodes + 1);
        int new_score = small_parsimony.get()->total_score;
        shared_ptr<string> new_string_list = small_parsimony->string_list;
        while(new_score < this->min_large_parsimony_score){
            this->min_large_parsimony_score = new_score;
            this->string_list = new_string_list;
            shared_ptr<int> edges = get_edges_from_unrooted_undirectional_tree();
            
            // For each edge, exchange the internal edges to get 2 new trees
            for(int i = 0; i < this->num_edges; i++){
                int a = edges.get()[i];
                int b = edges.get()[i + 1];
                for(int j = 0; j < 2; j++){ // exchange b's j_th child in unrooted & undirectional tree
                    nearest_neighbor_interchage(a, b, j); // writed to this->unrooted_undirectional_tree
                    make_tree_rooted_directional();// write to this->rooted_char_list; this->rooted_directional_tree; this->rooted_directional_idx_arr;
                    small_parsimony = make_shared<SmallParsimony>(this->rooted_directional_idx_arr, this->rooted_directional_tree, this->cur_rooted_char_list, this->num_char_trees, num_nodes + 1);
                    if(small_parsimony->total_score < new_score){// compare
                        new_score = small_parsimony->total_score;
                        new_string_list = small_parsimony->string_list;
                    }
                }
            }
        }
    }
};

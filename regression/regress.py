import os
import sys
import copy
import random
from collections import deque
sys.path.append('../parsimony_python')
from parsimony import *
from parsimony_util import *


def generate_tree_structure(num_leaves):
    T = {0: [1], 1: [0]}
 
    next_new_leave = 2
    cur_leaves = deque()
    cur_leaves.append(0)
    cur_leaves.append(1)
    
    while len(cur_leaves) < num_leaves:
        top_leave = cur_leaves.popleft()
        leave_1 = next_new_leave;
        leave_2 = next_new_leave + 1;
        next_new_leave += 2;

        T[top_leave].append(leave_1)
        T[top_leave].append(leave_2)
        T[leave_1] = [top_leave]
        T[leave_2] = [top_leave]

        cur_leaves.append(leave_1)
        cur_leaves.append(leave_2)

    # remapping the node number
    # make sure leaves are in [0, num_leaves)
    new_mapping = {}
    new_counter = 0
    new_T = {}
    for i, j in T.items():
        if len(j) == 1:
            new_mapping[i] = new_counter
            new_counter += 1
    for i, j in T.items():
        if i not in new_mapping:
            new_mapping[i] = new_counter
            new_counter += 1
    for i, j in T.items():
        new_T[new_mapping[i]] = []
        for k in j:
            new_T[new_mapping[i]].append(new_mapping[k])

    return new_T


def generate_random_str(str_len):
    result = []
    candidates = ["A", "C", "G", "T"]
    for i in range(str_len):
        result.append(candidates[random.randint(0, 3)])
    return "".join(result)


def assign_strings(T, str_len):
    assignment = {}
    for i, j in T.items():
        if len(j) == 1:
            assignment[i] = generate_random_str(str_len)
    return assignment


def generate_input_file(T, assignment, file_name):
    num_leaves = len(assignment)

    with open(file_name, 'w') as outputfile:
        outputfile.write("{}\n".format(num_leaves))
        for i, j in T.items():
            for k in j:
                if i in assignment:
                    i = assignment[i]
                if k in assignment:
                    k = assignment[k]
                outputfile.write("{}->{}\n".format(i, k))


def run_python_baseline(file_name, outfile_name):
    T = {}
    assign = {}
    counter = [0]
    character_list = []

    lines = read_lines(file_name)
    n = int(lines[0])

    for line in lines[1:]:
        father, son = get_neighbor_pair(line, assign, counter)
        connect_neighbor_pair(father, son, T)

    construct_char_list(character_list, assign)

    insert_root(T)

    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    result = nearest_neighbor_interchage(
        T, character_list)

    # write_result(outfile_name, result)


def get_tree_char(trees_list):
    T_list = []

    for each_tree in trees_list:
        T = {}
        node_chars_map = {}

        score = int(each_tree[0])
        each_tree = each_tree[1:]

        for i in range(len(each_tree)):
            each = each_tree[i]
            pair = each.split("->")
            if not pair[1].isdigit():
                break
            father = int(pair[0])
            son = int(pair[1])
            connect_neighbor_pair(father, son, T)

        for j in range(i, len(each_tree)):
            each = each_tree[j]
            pair = each.split("->")
            node = int(pair[0])
            chars = pair[1]
            node_chars_map[node] = chars
        T_list.append((score, T, node_chars_map))

    return T_list


def validate_tree(tree_tuple):
    score = tree_tuple[0]
    T = tree_tuple[1]
    chars = tree_tuple[2]

    char_len = len(chars[0])
    for node, each_char in chars.items():
        if len(each_char) != char_len:
            return False, "Char Length Mismatch"

    visited = set()
    tree_score = 0
    for i in T:
        for j in T[i]:
            if not (i in visited and j in visited):
                tree_score += hamming_distance(chars[i], chars[j])
                visited.add(i)
                visited.add(j)

    if tree_score != score:
        return False, "Score Mismatch: min_score[{}] tree_score[{}]".format(score, tree_score)

    return True, "Valid Tree"


def compare_two_files(file_1_name, file_2_name):
    trees_1 = []
    trees_2 = []

    with open(file_1_name, 'r') as input_file:
        tree = []
        for line in input_file:
            line = line.strip()
            if line != "-----":
                tree.append(line)
            else:
                trees_1.append(tree)
                tree = []

    with open(file_2_name, 'r') as input_file:
        tree = []
        for line in input_file:
            line = line.strip()
            if line != "-----":
                tree.append(line)
            else:
                trees_2.append(tree)
                tree = []

    T_list_1 = get_tree_char(trees_1)
    T_list_2 = get_tree_char(trees_2)
    file_1_score = T_list_1[0][0]
    file_2_score = T_list_2[0][0]

    if file_1_score != file_2_score:
        return False, "Minimum Score Mismatch: file_1 [{}] file_2 [{}]".format(file_1_score, file_2_score)

    for each in T_list_1:
        result, error_info = validate_tree(each)
        if not result:
            return result, error_info

    for each in T_list_2:
        result, error_info = validate_tree(each)
        if not result:
            return result, error_info

    return True, "Match"


def main():
    new_input_file = "../data/test_input.txt"
    input_file = "../data/dataset_38507_8.txt"
    python_outfile = "../output/python_result.txt"
    cpp_outfile = "../output/cpp_result.txt"

    num_leaves = 5
    str_len = 50
    tree = generate_tree_structure(num_leaves)
    assigment = assign_strings(tree, str_len)
    generate_input_file(tree, assigment, new_input_file)

    run_python_baseline(new_input_file, python_outfile)

    # result = compare_two_files(python_outfile, cpp_outfile)
    # print(result)
   

if __name__ == '__main__':
    main()

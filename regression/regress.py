import os
import sys
import copy
from collections import deque
sys.path.append('../parsimony_python')
from parsimony import *
from parsimony_util import *


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

    write_result(outfile_name, result)


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
        return False, "Minimum Score Mismatch: file_1 [{}] file_2 [{}]".format()

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
    input_file = "../data/dataset_38507_8.txt"
    python_outfile = "../output/python_result.txt"
    cpp_outfile = "../output/cpp_result.txt"

    run_python_baseline(input_file, python_outfile)

    result = compare_two_files(python_outfile, cpp_outfile)
    print(result)

if __name__ == '__main__':
    main()

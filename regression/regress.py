import os
import sys
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

    # TODO : add comparision logic


def main():
    input_file = "../data/dataset_38507_8.txt"
    python_outfile = "../output/python_result.txt"
    cpp_outfile = "../output/cpp_result.txt"

    run_python_baseline(input_file, python_outfile)

    compare_two_files(python_outfile, cpp_outfile)

if __name__ == '__main__':
    main()

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
    result_map = nearest_neighbor_interchage(
        T, character_list)

    write_result_map(outfile_name, result_map)


def main():
    input_file = "../data/dataset_38507_8.txt"
    python_outfile = "../output/python_result.txt"

    run_python_baseline(input_file, python_outfile)


if __name__ == '__main__':
    main()

import os
import sys
sys.path.append('../parsimony_python')
from parsimony import *


def get_neighbor_pair(line, assign, counter):
    division = line.strip().split("->")

    if division[0].isdigit() and division[1].isdigit():
        father = int(division[0])
        son = int(division[1])
    elif division[0].isdigit():
        father = int(division[0])
        b = division[1]
        if b not in assign:
            assign[b] = counter[0]
            counter[0] += 1
        son = assign[b]
    elif division[1].isdigit():
        a = division[0]
        son = int(division[1])
        if a not in assign:
            assign[a] = counter[0]
            counter[0] += 1
        father = assign[a]
    else:
        a = division[0]
        b = division[1]
        if a not in assign:
            assign[a] = counter[0]
            counter[0] += 1
        if b not in assign:
            assign[b] = counter[0]
            counter[0] += 1
        father = assign[a]
        son = assign[b]

    return father, son


def connect_neighbor_pair(father, son, T):
    if father in T:
        if son not in T[father]:
            T[father].append(son)
    else:
        T[father] = [son]

    if son in T:
        if father not in T[son]:
            T[son].append(father)
    else:
        T[son] = [father]


def construct_char_list(character_list, assign):
    for each in assign:
        while len(character_list) < len(each):
            character_list.append({})
        for i in range(len(each)):
            character_list[i][assign[each]] = each[i]


def insert_root(T):
    new_node = max([i for i in T]) + 1
    left = [i for i in T][0]
    right = T[left][0]

    if len(T[left]) == 1:
        del T[left]
    else:
        T[left] = [i for i in T[left] if i != right]

    if len(T[right]) == 1:
        del T[right]
    else:
        T[right] = [i for i in T[right] if i != left]

    T.setdefault(left, []).append(new_node)
    T.setdefault(right, []).append(new_node)
    T.setdefault(new_node, []).append(left)
    T[new_node].append(right)


def read_lines(file_name):
    result = []
    with open(file_name, "r") as data:
        for line in data:
            result.append(line.strip())
    return result


def write_result(file_name, result):
    with open(file_name, "w") as output:
        for i in result:
            output.write(i)


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


def main():
    input_file = "../data/dataset_38507_8.txt"
    python_outfile = "python_result.txt"
    cpp_outfile = "cpp_result.txt"

    run_python_baseline(input_file, python_outfile)


if __name__ == '__main__':
    main()

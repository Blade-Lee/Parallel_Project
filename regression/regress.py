import os
import sys
from parsimony import *


def main():
    #os.chdir("E:\\Google Drive\\My Workspace\\USA\\BIC\\Spring 2017 courses\\02604 Bio\\Coding Assignments")
    # os.chdir("C:\\Users\\Blade\\Google Drive\\My Workspace\\USA\\BIC\\Spring 2017 courses\\02604 Bio\\Coding Assignments")
    '''
    with open("Week 10\\dataset_38506_12.txt", "r") as data:
        n = int(data.readline().strip())
        T = {}
        assign = {}
        counter = 0
        character_list = []
        for line in data:
            division = line.strip().split("->")

            if division[0].isdigit() and division[1].isdigit():
                father = int(division[0])
                son = int(division[1])
            elif division[0].isdigit():
                father = int(division[0])
                b = division[1]
                if b not in assign:
                    assign[b] = counter
                    counter += 1
                son = assign[b]
            elif division[1].isdigit():
                a = division[0]
                son = int(division[1])
                if a not in assign:
                    assign[a] = counter
                    counter += 1
                father = assign[a]
            else:
                a = division[0]
                b = division[1]
                if a not in assign:
                    assign[a] = counter
                    counter += 1
                if b not in assign:
                    assign[b] = counter
                    counter += 1
                father = assign[a]
                son = assign[b]

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

        for each in assign:
            while len(character_list) < len(each):
                character_list.append({})
            for i in range(len(each)):
                character_list[i][assign[each]] = each[i]

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

    # Tree format
    # T = {node: {"str": xxx, "children": [...]}, ...}
    tree_list = []

    total = 0
    for each in character_list:
        result, backtrack_tree, root = small_parsimony_one_tree_unrooted(copy.deepcopy(T), each)
        tree_list.append(backtrack_tree)
        total += result

    combined_tree = {}

    store = deque()
    store.append(root)

    while len(store) > 0:
        cur = store.popleft()
        str_list = [each[cur]["str"] for each in tree_list]
        combined_tree.setdefault(cur, {})["str"] = "".join(str_list)
        combined_tree[cur]["children"] = tree_list[0][cur]["children"]
        for son in combined_tree[cur]["children"]:
            store.append(son)

    with open("result.txt", "w") as data:
        data.write("{}\n".format(total))
        store = deque()
        store.append(root)
        while len(store) > 0:
            cur = store.popleft()
            cur_str = combined_tree[cur]["str"]
            for son in combined_tree[cur]["children"]:
                son_str = combined_tree[son]["str"]
                data.write("{}->{}:{}\n".format(cur_str, son_str, hamming_distance(cur_str, son_str)))
                data.write("{}->{}:{}\n".format(son_str, cur_str, hamming_distance(cur_str, son_str)))
                store.append(son)
    '''

    '''
    with open("Week 10\\dataset_38507_6.txt", "r") as data:
        options = data.readline().strip().split(" ")
        a = int(options[0])
        b = int(options[1])
        T = {}
        for line in data:
            cur = line.strip().split("->")
            father = int(cur[0])
            son = int(cur[1])
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

    T_1, T_2 = nearest_neighbors(a, b, T)

    with open("result.txt", "w") as output:
        for i in T_1:
            for j in T_1[i]:
                output.write("{}->{}\n".format(i, j))
        output.write("\n")
        for i in T_2:
            for j in T_2[i]:
                output.write("{}->{}\n".format(i, j))
    '''

    with open("dataset_38507_8.txt", "r") as data:
        n = int(data.readline().strip())
        T = {}
        assign = {}
        counter = 0
        character_list = []
        for line in data:
            division = line.strip().split("->")

            if division[0].isdigit() and division[1].isdigit():
                father = int(division[0])
                son = int(division[1])
            elif division[0].isdigit():
                father = int(division[0])
                b = division[1]
                if b not in assign:
                    assign[b] = counter
                    counter += 1
                son = assign[b]
            elif division[1].isdigit():
                a = division[0]
                son = int(division[1])
                if a not in assign:
                    assign[a] = counter
                    counter += 1
                father = assign[a]
            else:
                a = division[0]
                b = division[1]
                if a not in assign:
                    assign[a] = counter
                    counter += 1
                if b not in assign:
                    assign[b] = counter
                    counter += 1
                father = assign[a]
                son = assign[b]

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

        for each in assign:
            while len(character_list) < len(each):
                character_list.append({})
            for i in range(len(each)):
                character_list[i][assign[each]] = each[i]

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

    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    result = nearest_neighbor_interchage(T, character_list)

    with open("result.txt", "w") as output:
        for i in result:
            output.write(i)

if __name__ == '__main__':
    main()

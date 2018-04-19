import copy
from collections import deque


def find_ripe_node_one_tree_unrooted(T, tag):
    for each in T:
        if tag[each] == 0:
            count = 0
            children = []
            for child in T[each]:
                if tag[child] == 1:
                    count += 1
                    children.append(child)
            if count == 2:
                return True, each, children
    return False, None, [], []


def hamming_distance(a, b):
    n = len(a)
    count = 0
    for i in range(n):
        if a[i] != b[i]:
            count += 1
    return count


def check(a, b):
    if a == b:
        return 0
    return 1


# T is undirected
def small_parsimony_one_tree_unrooted(T, character):
    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    backtrack = {}
    s_k_v = {}
    tag = {}
    for v in T:
        if len(T[v]) == 1:
            tag[v] = 1
            for k in {"A", "T", "C", "G"}:
                if character.get(v, "Q") == k:
                    s_k_v.setdefault(v, {})[k] = 0
                else:
                    s_k_v.setdefault(v, {})[k] = 100000000
        else:
            tag[v] = 0

    v = None
    while True:
        option = find_ripe_node_one_tree_unrooted(T, tag)
        if not option[0]:
            break

        v = option[1]
        tag[v] = 1

        daughter = option[2][0]
        son = option[2][1]

        for k in {"A", "T", "C", "G"}:

            backtrack.setdefault(v, {})[k] = {}

            first = 1000000000
            second = 1000000000

            for cur_daughter in {"A", "T", "C", "G"}:
                temp = s_k_v[daughter][cur_daughter] + check(cur_daughter, k)
                if temp < first:
                    first = temp
                    min_daughter = cur_daughter

            for cur_son in {"A", "T", "C", "G"}:
                temp = s_k_v[son][cur_son] + check(cur_son, k)
                if temp < second:
                    second = temp
                    min_son = cur_son

            s_k_v.setdefault(v, {})[k] = first + second

            backtrack[v][k][daughter] = min_daughter
            backtrack[v][k][son] = min_son

    result = 1000000000
    for k in s_k_v[v]:
        if s_k_v[v][k] < result:
            result = s_k_v[v][k]
            first_str = k

    left = T[v][0]
    right = T[v][1]
    left_str = backtrack[v][first_str][left]
    right_str = backtrack[v][first_str][right]

    backtrack.setdefault(left, {}).setdefault(left_str, {})[right] = right_str
    backtrack.setdefault(right, {}).setdefault(right_str, {})[left] = left_str

    del T[v]

    T[left] = [i for i in T[left] if i != v]
    T[left].append(right)
    T[right] = [i for i in T[right] if i != v]
    T[right].append(left)

    v = left
    first_str = left_str

    # New Tree format, undirected, unrooted
    # new_tree = {node: {"str": xxx, "children": [...]}, ...}

    new_tree = {v: {"str": first_str, "children": []}}

    store = deque()
    new_store = deque()
    store.append((v, first_str))
    new_store.append(v)
    visited = set()

    while len(store) > 0:
        options = store.popleft()
        father = options[0]
        father_str = options[1]
        new_tree[father]["children"] = T[father]
        if father == v or len(T[father]) > 1:
            for son in T[father]:
                if son in backtrack[father][father_str] and son not in visited:
                    son_str = backtrack[father][father_str][son]
                    new_tree[son] = {"str": son_str, "children": []}
                    store.append((son, son_str))
                    new_store.append(son)

        visited.add(father)

    return result, new_tree, v


def switch_subtree(a, x, b, y, T):

    if y is not None:
        T[a].append(y)
        T[y].append(a)
        T[b] = [i for i in T[b] if i != y]
        T[y] = [i for i in T[y] if i != b]

    if x is not None:
        T[b].append(x)
        T[x].append(b)
        T[a] = [i for i in T[a] if i != x]
        T[x] = [i for i in T[x] if i != a]

    return T


def switch_subtree_combined(a, x, b, y, T):

    if y is not None:
        T[a]["children"].append(y)
        T[y]["children"].append(a)
        T[b]["children"] = [i for i in T[b]["children"] if i != y]
        T[y]["children"] = [i for i in T[y]["children"] if i != b]

    if x is not None:
        T[b]["children"].append(x)
        T[x]["children"].append(b)
        T[a]["children"] = [i for i in T[a]["children"] if i != x]
        T[x]["children"] = [i for i in T[x]["children"] if i != a]

    return T


def nearest_neighbors(a, b, T):

    T_1 = copy.deepcopy(T)
    T_2 = copy.deepcopy(T)

    a_candidates = [i for i in T[a] if i != b]
    b_candidates = [i for i in T[b] if i != a]

    '''
    if len(a_candidates) == 0 and len(b_candidates) == 0:
        pass
    elif len(a_candidates) == 2 and len(b_candidates) == 0:
        T_1 = switch_subtree(a, a_candidates[0], b, None, T_1)
        T_2 = switch_subtree(a, a_candidates[1], b, None, T_2)
    elif len(a_candidates) == 0 and len(b_candidates) == 2:
        T_1 = switch_subtree(a, None, b, b_candidates[0], T_1)
        T_2 = switch_subtree(a, None, b, b_candidates[1], T_2)
    else:
        T_1 = switch_subtree(a, a_candidates[0], b, b_candidates[0], T_1)
        T_2 = switch_subtree(a, a_candidates[0], b, b_candidates[1], T_2)
    '''

    if len(a_candidates) == 2 and len(b_candidates) == 2:
        T_1 = switch_subtree(a, a_candidates[0], b, b_candidates[0], T_1)
        T_2 = switch_subtree(a, a_candidates[0], b, b_candidates[1], T_2)

    return T_1, T_2


def nearest_neighbors_combined(a, b, T):

    T_1 = copy.deepcopy(T)
    T_2 = copy.deepcopy(T)

    a_candidates = [i for i in T[a]["children"] if i != b]
    b_candidates = [i for i in T[b]["children"] if i != a]

    '''
    if len(a_candidates) == 0 and len(b_candidates) == 0:
        pass
    elif len(a_candidates) == 2 and len(b_candidates) == 0:
        T_1 = switch_subtree(a, a_candidates[0], b, None, T_1)
        T_2 = switch_subtree(a, a_candidates[1], b, None, T_2)
    elif len(a_candidates) == 0 and len(b_candidates) == 2:
        T_1 = switch_subtree(a, None, b, b_candidates[0], T_1)
        T_2 = switch_subtree(a, None, b, b_candidates[1], T_2)
    else:
        T_1 = switch_subtree(a, a_candidates[0], b, b_candidates[0], T_1)
        T_2 = switch_subtree(a, a_candidates[0], b, b_candidates[1], T_2)
    '''

    if len(a_candidates) == 2 and len(b_candidates) == 2:
        T_1 = switch_subtree_combined(
            a, a_candidates[0], b, b_candidates[0], T_1)
        T_2 = switch_subtree_combined(
            a, a_candidates[0], b, b_candidates[1], T_2)

    return T_1, T_2


def run_small_parsimony_one_tree_unrooted(T, character_list):
    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    # backtrack tree format, undirected, unrooted
    # backtrack_tree = {node: {"str": xxx, "children": [...]}, ...}
    tree_list = []

    # print "Small input tree"
    # for i, j in T.iteritems():
    #    print "{} : {}".format(i, j)

    total = 0
    for each in character_list:
        # backtrack_tree is undirected
        result, backtrack_tree, root = small_parsimony_one_tree_unrooted(
            copy.deepcopy(T), each)
        tree_list.append(backtrack_tree)
        total += result

    combined_tree = {}

    store = deque()
    store.append(root)
    visited = set()

    while len(store) > 0:
        cur = store.popleft()
        str_list = [each[cur]["str"] for each in tree_list]
        combined_tree.setdefault(cur, {})["str"] = "".join(str_list)
        combined_tree[cur]["children"] = tree_list[0][cur]["children"]
        for son in combined_tree[cur]["children"]:
            if son not in visited:
                store.append(son)
        visited.add(cur)

    output = ["{}\n".format(total)]

    store = deque()
    store.append(root)
    visited = set()

    while len(store) > 0:
        cur = store.popleft()
        cur_str = combined_tree[cur]["str"]
        for son in combined_tree[cur]["children"]:
            son_str = combined_tree[son]["str"]
            output.append("{}->{}:{}\n".format(cur_str, son_str,
                                               hamming_distance(cur_str, son_str)))
            #output.append("{}->{}:{}\n".format(son_str, cur_str, hamming_distance(cur_str, son_str)))
            if son not in visited:
                store.append(son)
        visited.add(cur)

    # print "Small output tree"
    # for i, j in combined_tree.iteritems():
    #    print "{} : {}".format(i, j)
    # print

    return total, output, combined_tree


def decompose_combined_tree(combined_tree):

    # combined_tree: undirected, unrooted
    # Combined Tree format
    # combined_tree = {node: {"str": xxx, "children": [...]}, ...}
    # nodes are fully assigned with strings

    assign = {}
    temp_T = {i: combined_tree[i]["children"] for i in combined_tree}

    new_node = max([i for i in temp_T]) + 1
    left = [i for i in temp_T][0]
    right = temp_T[left][0]

    if len(temp_T[left]) == 1:
        del temp_T[left]
    else:
        temp_T[left] = [i for i in temp_T[left] if i != right]

    if len(temp_T[right]) == 1:
        del temp_T[right]
    else:
        temp_T[right] = [i for i in temp_T[right] if i != left]

    temp_T.setdefault(left, []).append(new_node)
    temp_T.setdefault(right, []).append(new_node)
    temp_T.setdefault(new_node, []).append(left)
    temp_T[new_node].append(right)

    # temp_T is rooted now

    for i in combined_tree:
        assign[combined_tree[i]['str']] = i

    character_list = []

    for each in assign:
        while len(character_list) < len(each):
            character_list.append({})
        for i in range(len(each)):
            if len(temp_T[assign[each]]) == 1:
                character_list[i][assign[each]] = each[i]

    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    return temp_T, character_list


def nearest_neighbor_interchage(T, character_list):
    # T is undirected, rooted
    # format: T : {a : [b, c, d], b: [a], ...}
    # Combined Tree format, undirected, unrooted
    # combined_tree = {node: {"str": xxx, "children": [...]}, ...}

    total_output = []

    score = 1000000000

    new_score, output, new_tree = run_small_parsimony_one_tree_unrooted(
        T, character_list)

    firstone = True
    temp_output = output
    while new_score < score:
        score = new_score
        tree = new_tree
        if not firstone:
            for i in temp_output:
                total_output.append(i)
            total_output.append('\n')
        if firstone:
            firstone = False

        Tree_edges = set()
        store = deque()
        store.append(0)
        visited = set()

        while len(store) > 0:
            cur = store.popleft()
            for son in tree[cur]["children"]:
                if son not in visited:
                    if len(tree[cur]["children"]) > 1 and len(tree[son]["children"]) > 1:
                        Tree_edges.add((cur, son))
                    store.append(son)
            visited.add(cur)

        for e in Tree_edges:
            neighbors = nearest_neighbors_combined(e[0], e[1], tree)
            for NeighborTree in neighbors:
                rooted_tree, character_list = decompose_combined_tree(
                    NeighborTree)
                neighborScore, output, temp_tree = run_small_parsimony_one_tree_unrooted(
                    rooted_tree, character_list)
                if neighborScore < new_score:
                    new_score = neighborScore
                    new_tree = temp_tree
                    temp_output = output

    return total_output

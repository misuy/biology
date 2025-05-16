#!/usr/bin/python3

import json
from sys import argv
import numpy as np
import scipy
import scipy.optimize

# struct blgy_prxy_bio_info {
#     int cpu;
#     uint32_t id;
#     long long start_ts;
#     long long end_ts;
#     unsigned long long sector;
#     unsigned int size;
#     int op;
#     unsigned char status;
#     struct blgy_prxy_bio_info_payload payload;
# };


scale = {
    "cpu": 1,
    "op": 5,
    "sector": 3,
    "size": 2,
    "start_ts": 2,
}

def max_value(reqs, key):
    return max([req[key] for req in reqs])

def min_value(reqs, key):
    return min([req[key] for req in reqs])

def max_delta_key(reqs0, reqs1, key):
    return max(abs(min_value(reqs0, key) - max_value(reqs1, key)), abs(min_value(reqs1, key) - max_value(reqs0, key)))

def max_delta(reqs0, reqs1):
    return {
        "cpu": 1,
        "op": 1,
        "sector": max_delta_key(reqs0, reqs1, "sector"),
        "size": max_delta_key(reqs0, reqs1, "size"),
        "start_ts": max_delta_key(reqs0, reqs1, "start_ts"),
    }

def generic_distance(req0, reqs1, max_delta, key):
    value0 = req0[key]
    values1 = np.array([req[key] for req in reqs1])
    diff = np.abs(value0 - values1)
    if max_delta[key] != 0:
        return (diff / max_delta[key]) * scale[key]

    return diff * scale[key]

def binary_distance(req0, reqs1, key):
    value0 = req0[key]
    values1 = np.array([req[key] for req in reqs1])
    return np.not_equal(value0, values1).astype(int) * scale[key]

def distance(req0, reqs1, max_delta):
    if (req0 == None):
        return np.full(len(reqs1), 1)

    cpu = binary_distance(req0, reqs1, "cpu")
    op = binary_distance(req0, reqs1, "op")
    sector = generic_distance(req0, reqs1, max_delta, "sector")
    size = generic_distance(req0, reqs1, max_delta, "size")
    start_ts = generic_distance(req0, reqs1, max_delta, "start_ts")

    max_distance = np.sqrt(np.sum(np.array([sc for sc in scale.values()]) ** 2))
    return np.sqrt(cpu ** 2 + start_ts ** 2 + sector ** 2 + size ** 2 + op ** 2) / max_distance

def distance_matrix(reqs0, reqs1):
    delta = max_delta(reqs0, reqs1)

    if len(reqs0) > len(reqs1):
        reqs0, reqs1 = reqs1, reqs0
        reqs0.extend([None] * (len(reqs1) - len(reqs0)))

    m = np.zeros((len(reqs0), len(reqs1)))

    for i, req0 in enumerate(reqs0):
        m[i, :] = distance(req0, reqs1, delta)

    return m


def hungarian_algorithm(cost_matrix):
    cost_matrix = np.array(cost_matrix)
    n = cost_matrix.shape[0]
    m = cost_matrix.shape[1]

    if n != m:
        raise ValueError("Матрица стоимости должна быть квадратной")

    for i in range(n):
        cost_matrix[i] -= np.min(cost_matrix[i])

    for j in range(n):
        cost_matrix[:, j] -= np.min(cost_matrix[:, j])

    row_matches = [-1] * n
    col_matches = [-1] * n
    for i in range(n):
        for j in range(n):
            if cost_matrix[i, j] == 0 and col_matches[j] == -1:
                row_matches[i] = j
                col_matches[j] = i
                break

    unassigned_rows = [i for i in range(n) if row_matches[i] == -1]

    while unassigned_rows:
        marked_rows = [False] * n
        marked_cols = [False] * n
        start_row = unassigned_rows[0]
        marked_rows[start_row] = True
        path = []

        found_augmenting_path = False
        while not found_augmenting_path:
            for j in range(n):
                if cost_matrix[start_row, j] == 0 and not marked_cols[j]:
                    marked_cols[j] = True
                    path.append((start_row, j))

                    if col_matches[j] == -1:
                        found_augmenting_path = True
                        break
                    else:
                        row = col_matches[j]
                        marked_rows[row] = True
                        start_row = row
                        path.append((j, row))

            if not found_augmenting_path:
                min_val = np.inf
                for i in range(n):
                    for j in range(n):
                        if marked_rows[i] and not marked_cols[j]:
                            min_val = min(min_val, cost_matrix[i, j])

                for i in range(n):
                    for j in range(n):
                        if marked_rows[i]:
                            cost_matrix[i, j] -= min_val
                        if marked_cols[j]:
                            cost_matrix[i, j] += min_val

                start_row = unassigned_rows[0]
                marked_rows = [False] * n
                marked_cols = [False] * n
                marked_rows[start_row] = True
                path = []

        for i in range(0, len(path), 2):
            row, col = path[i][0], path[i][1]
            row_matches[row] = col
            col_matches[col] = row

        unassigned_rows = [i for i in range(n) if row_matches[i] == -1]


    assignment = [(i, row_matches[i]) for i in range(n)]
    min_cost = np.sum([cost_matrix[i, row_matches[i]] for i in range(n)])

    return assignment, min_cost


if len(argv) < 3:
    print("err: bad input (expected ./stat.py {dump-file-0.json} {dump-file-1.json})")

use_hungarian = 0
if len(argv) == 4:
    if int(argv[3] == 1):
        use_hungarian = 1

f0 = open(argv[1], 'r')
f1 = open(argv[2], 'r')

data0 = json.load(f0)
reqs0 = data0["requests"]

data1 = json.load(f1)
reqs1 = data1["requests"]

m = distance_matrix(reqs0, reqs1)

if (use_hungarian):
    assignment, cost = hungarian_algorithm(m)
else:
    row, col = scipy.optimize.linear_sum_assignment(m)
    cost = m[row, col].sum()

print(1 - cost / m.shape[0])

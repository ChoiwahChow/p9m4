#!/usr/bin/python3

"""
Complete cube-and-conquer with bootstrapping

Iterative:
1. lengthen cubes by parallel Mace4
2. remove isomorphic cubes
3. repeat until the desired length of cubes is reached

Then
4. run the cubes


Mace4 options:
-O3   strictly follows the cube ordering
-A1   print the models in the format expected by isofilter


grep "Exiting with " semi_working_[0-9]/mace.log | grep model | utils/mace4/counter.py

"""


import os
import time
import subprocess
import threading
import ast
from itertools import permutations

import extend_cubes
import multi_cube_analyzer as analyzer
import run_cubes

mace4_exe = "../build/mace4"
top_data_dir = "utils/mace4/working"
num_threads  = 8

order = 8
prev_cube_length = 32
cube_length = 50
print_models = "P0"  # P0 - don't output models, A1 - output models

cube_sequence_2 = [2, 4, 9, 16, 25, 36]
cube_sequence_1_2 = [2, 4, 6, 12, 20, 30, 42]
cube_sequence_2_2 = [2, 4, 8, 18, 32, 50, 72]

cube0_2 = [[0, 0], [0, 1], [0, 2], [1, 0], [1, 1], [1, 2]]
cube0_1_2 = [[0, 0], [0, 1], [1, 0], [1, 1], [1, 2]]
cube0_2_2 = cube0_1_2

id_permuation = []
permutations2 = [[1, 0]]
permutations3 = permutations2 + [[0,2,1],[1,2,0],[2,1,0],[2,0,1]]
# permutations = [[0, 2, 1]]
permutations4 =  analyzer.shorten_permutations(list(permutations(range(0, 4))))   # permute 0, 1, 2, 3 only
permutations5 = analyzer.shorten_permutations(list(permutations(range(0, 5))))
permutations6 = analyzer.shorten_permutations(list(permutations(range(0, 6))))
perm = {0: id_permuation, 1: permutations2, 2: permutations3, 3: permutations4, 4: permutations5, 5: permutations6}

# calculate radii
r_2 = {(x+1)**2: x for x in range(0, 6)} # 1 binary op {1: 0, 4: 1, 9: 2, 16: 3
r_2.update({2 : 1})
r_2_2 = {k*2: v for k, v in r_2.items()}   # 2 binary op
r_2_2_2 = {k*3: v for k, v in r_2.items()}   # 3 binary op
r_1_2   = {k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
    
run_data = {'quandles': {'seq': cube_sequence_2_2, 'relations': [False, False], 'cube0': cube0_2_2, 
                         'input': '87_quandles', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1}}


def get_data_dir(algebra, order):
    return f"{top_data_dir}/{algebra}{order}"


def get_working_dir(algebra, order, cube_length):
    return f"{algebra}_working_{order}_{cube_length}"
    

def collect_cubes(algebra, cube_dir, prev_cube_length, cube_length, data_dir):
    al = run_data[algebra]
    cmd = f"cat {cube_dir}_*/{cube_length}.out | grep '^cube' | sort | uniq | sed 's/[^ ]* //' > {data_dir}/cubes{cube_length}.out"
    subprocess.run(cmd, capture_output=False, text=True, check=False, shell=True)
    cube_file = f"{data_dir}/cubes{cube_length}.out"
    prev_file = f"{data_dir}/cubes_{order}_{prev_cube_length}.out"
    out_cube_file = f"{data_dir}/cubes_{order}_{cube_length}.out"
    
    all_permutations = perm[al['radius'][cube_length]]
    if al['remove'] == 0:
        all_permutations = analyzer.remove0(all_permutations)
        
    analyzer.gen_sequence(order, cube_length, al['arities'], al['relations'], all_permutations, prev_file, cube_file, out_cube_file)
    

def gen_all_cubes(algebra, order, target_cube_length):
    data_dir = get_data_dir(algebra, order)
    os.makedirs(data_dir, exist_ok=True)

    seq = run_data[algebra]['seq']
    cube_length = 0
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    for new_cube_length in seq:
        new_cube_dir = get_working_dir(algebra, order, new_cube_length)
        if new_cube_length == 2:
            cube_file = None
        else:
            cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{cube_length}.out"
        extend_cubes.extend_cubes(input_file, order, new_cube_length, cube_file, print_models, mace4_exe, new_cube_dir, num_threads)
        collect_cubes(algebra, new_cube_dir, cube_length, new_cube_length, data_dir)
        if new_cube_length == target_cube_length:
            break
        cube_length = new_cube_length
    
    
def run_all_cubes(algebra, order, target_cube_length):    
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{target_cube_length}.out"
    working_dir_prefix = get_working_dir(algebra, order, target_cube_length)
    run_cubes.run_mace(input_file, order, cube_file, print_models, working_dir_prefix, num_threads)
    
    cmd = f'grep "Exiting with " {working_dir_prefix}_*/mace.log | grep model | utils/mace4/counter.py'
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    return int(sp.stdout)

    
def collect_stat(algebra, order, target_cube_length, models_count, gen_cube_time, runtime):
    data_dir = get_data_dir(algebra, order)
    out_cube_file = f"{data_dir}/cubes_{order}_{cube_length}.out"
    count = len(open(out_cube_file).readlines( ))
    print(f'"{algebra}", {order}, {count}, {gen_cube_time}, {models_count}, {runtime}\n')


__all__ = ["run_all"]

if __name__ == "__main__":
    algebra = "quasi"
    algebra = "hilbert"
    algebra = "quasi_ordered"
    algebra = "loops"
    algebra = "tarski"
    algebra = "semizero"
    algebra = "semi"
    algebra = "inv_semi"
    algebra = "quandles"    # order 10

    target_cube_length = 32
    order = 10

    t0 = time.time()
    gen_all_cubes(algebra, order, target_cube_length)
    t1 = time.time()
    gen_cube_time = t1 - t0
    t2 = time.time()
    print(f"Done generating cubes. Generate models...")
    models_count = run_all_cubes(algebra, order, target_cube_length)
    t3 = time.time()
    runtime = t3 - t2
    collect_stat(algebra, order, target_cube_length, models_count, gen_cube_time, runtime)
    

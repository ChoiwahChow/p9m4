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

top_data_dir = "utils/mace4/working"
num_threads  = 8


request_work_file = "request_work.txt"
work_file = "release_work.out"
print_models = "P0"  # P0 - don't output models, A1 - output models

cube_sequence_2 = [2, 4, 9, 16, 25, 36, 49]   # for radius 2, 2, 3, 4, 5, 6, 7, ...
cube_sequence_1_2 = [2, 4, 6, 12, 20, 30, 42, 56]   # for raduis 1, 2, 2, 3, 4, 5, 6...
cube_sequence_1_2_2 = [3, 6, 10, 21, 36, 55, 78, 105]
cube_sequence_2_2 = [2, 4, 8, 18, 32, 50, 72, 98]
cube_sequence_2_2_2 = [3, 6, 12, 27, 48, 75, 108, 147]
cube_sequence_1_2_2_2 = [4, 8, 14, 30, 52, 80, 114, 154]
cube_sequence_2_2_2_2 = [4, 8, 16, 36, 64, 100, 144, 196]
cube_sequence_1_2_2_2_2 = [5, 10, 18, 39, 68, 105, 150, 203]

id_permuation = []
permutations2 = [[1, 0]]
permutations3 = permutations2 + [[0,2,1],[1,2,0],[2,1,0],[2,0,1]]
permutations4 =  analyzer.shorten_permutations(list(permutations(range(0, 4))))   # permute 0, 1, 2, 3 only
permutations5 = analyzer.shorten_permutations(list(permutations(range(0, 5))))
permutations6 = analyzer.shorten_permutations(list(permutations(range(0, 6))))
permutations7 = analyzer.shorten_permutations(list(permutations(range(0, 7))))
perm = {0: id_permuation, 1: permutations2, 2: permutations3, 3: permutations4, 4: permutations5, 5: permutations6, 6: permutations7}

# calculate radii
r_2 = {(x+1)**2: x for x in range(0, 7)} # 1 binary op {1: 0, 4: 1, 9: 2, 16: 3
r_2.update({2 : 1})
r_2_2 = {k*2: v for k, v in r_2.items()}   # 2 binary op
r_2_2_2 = {k*3: v for k, v in r_2.items()}   # 3 binary op
r_1_2   = {k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
r_1_2_2 = {2*k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
r_1_2_2_2   = {3*k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
r_2_2_2_2 = {k*4: v for k, v in r_2.items()}   # 4 binary op
r_1_2_2_2_2 = {k*4+v+1: v for k, v in r_2.items()}   # 1 unary op, 4 binary op
    
run_data = {'dist_lattice_ord_semi': {'seq': cube_sequence_2_2_2, 'relations': [False, False, False], 
                         'input': '36_dist_lattice_ord_semi', 'arities': [2, 2, 2], 'radius': r_2_2_2, 'remove': -1},
            'invol_lattices': {'seq': cube_sequence_1_2_2, 'relations': [False, False, False], 
                         'input': '50_invol_lattices', 'arities': [1, 2, 2], 'radius': r_1_2_2, 'remove': -1},
            'inv_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '121_inv_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': -1},
            'quandles': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '87_quandles', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'semi':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi', 'arities': [2], 'radius': r_2, 'remove': -1},
            'loops':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '32_loop', 'arities': [2], 'radius': r_2, 'remove': 0},
            'm_zeriods':   {'seq': cube_sequence_1_2_2_2_2, 'relations': [False, False, False, False, True], 
                         'input': '58_m_zeroids', 'arities': [1, 2, 2, 2, 2], 'radius': r_1_2_2_2_2, 'remove': 0},
            'moufang': {'seq': cube_sequence_2_2_2, 'relations': [False, False, False], 
                         'input': '64_moufang_quasi', 'arities': [2, 2, 2], 'radius': r_2_2_2, 'remove': 0},
            'ord_semilattice': {'seq': cube_sequence_2_2, 'relations': [False, True], 
                         'input': '78_ord_semilattice', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'order_algebras':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '74_order_algebras', 'arities': [2], 'radius': r_2, 'remove': -1},
            'posets':   {'seq': cube_sequence_2, 'relations': [True], 
                         'input': '86_posets', 'arities': [2], 'radius': r_2, 'remove': -1},
            'res_po_monoid':   {'seq': cube_sequence_2_2_2_2, 'relations': [False, False, False, True], 
                         'input': '91_resi_po_monoid', 'arities': [2, 2, 2, 2], 'radius': r_2_2_2_2, 'remove': 0},
            'tarski':   {'seq': cube_sequence_2, 'relations': [False],
                         'input': 'tarski', 'arities': [2], 'radius': r_2, 'remove': -1},
            'trigroup': {'seq': cube_sequence_1_2_2_2, 'relations': [False, False, False, False], 
                         'input': '142_trigroup', 'arities': [1, 2, 2, 2], 'radius': r_1_2_2_2, 'remove': 0}}


def get_data_dir(algebra, order):
    return f"{top_data_dir}/{algebra}{order}"


def get_working_dir(algebra, order, cube_length):
    """ compose working dir
    """
    return f"{algebra}_working_{order}_{cube_length}"
    

def collect_cubes(algebra, cube_dir, prev_cube_length, cube_length, threshold, data_dir):
    al = run_data[algebra]
    cmd = f"cat {cube_dir}_*/{cube_length}.out | grep '^cube' | sort | uniq | sed 's/[^ ]* //' > {data_dir}/cubes{cube_length}.out"
    subprocess.run(cmd, capture_output=False, text=True, check=False, shell=True)
    cube_file = f"{data_dir}/cubes{cube_length}.out"
    prev_file = f"{data_dir}/cubes_{order}_{prev_cube_length}.out"
    out_cube_file = f"{data_dir}/cubes_{order}_{cube_length}.out"
    
    radius = al['radius'][cube_length]
    all_permutations = perm[radius]
    if al['remove'] == 0:
        all_permutations = analyzer.remove0(all_permutations)
        
    analyzer.gen_sequence(order, cube_length, radius, al['arities'], al['relations'], all_permutations, threshold, prev_file, cube_file, out_cube_file)
    

def gen_all_cubes(algebra, order, target_cube_length, threshold, mace4_exe):
    """
    Args:
        algebra (str): name of the algebra
        order (int): order of the algebra
        target_cube_length (int): target cube length
        threshold (int):  use invariant when the number of models is at least this manany
        mace4_exe (str): path name of the mace4 executable
    """
    data_dir = get_data_dir(algebra, order)
    os.makedirs(data_dir, exist_ok=True)

    seq = run_data[algebra]['seq']
    cube_length = 0
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    for index, new_cube_length in enumerate(seq):
        new_cube_dir = get_working_dir(algebra, order, new_cube_length)
        if index == 0:
            cube_file = None
        else:
            cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{cube_length}.out"
        extend_cubes.extend_cubes(input_file, order, new_cube_length, cube_file, "P0", mace4_exe,
                                  new_cube_dir, num_threads, request_work_file, work_file)
        collect_cubes(algebra, new_cube_dir, cube_length, new_cube_length, threshold, data_dir)
        if new_cube_length == target_cube_length:
            break
        cube_length = new_cube_length
    
    
def run_all_cubes(algebra, order, target_cube_length, mace4_exe):    
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{target_cube_length}.out"
    working_dir_prefix = get_working_dir(algebra, order, target_cube_length)
    run_cubes.run_mace(mace4_exe, input_file, order, cube_file, print_models, working_dir_prefix, num_threads)
    
    cmd = f'grep "Exiting with " {working_dir_prefix}_*/mace.log | grep model | utils/mace4/counter.py'
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    return int(sp.stdout)

    
def collect_stat(algebra, order, target_cube_length, models_count, gen_cube_time, runtime):
    data_dir = get_data_dir(algebra, order)
    out_cube_file = f"{data_dir}/cubes_{order}_{target_cube_length}.out"
    count = len(open(out_cube_file).readlines( ))
    print(f'"{algebra}", {order}, {count}, {gen_cube_time}, {models_count}, {runtime}\n')


__all__ = ["run_all_cubes", "gen_all_cubes", "collect_stat"]

if __name__ == "__main__":
    mace4_exe = "../bin/mace4"
    threshold = 1000000

    algebra = "quasi"
    algebra = "hilbert"
    algebra = "semizero"
    algebra = "quandles"    # order 10
    algebra = "semi"
    algebra = "trigroup"
    algebra = "res_po_monoid"
    algebra = "semi"
    algebra = "m_zeriods"        # #58
    algebra = "inv_semi"         # #121 cube length 20 order 9
    algebra = "order_algebras"   # #74  cube length  25, order 8
    algebra = "ord_semilattice"  # #78  cube length 72, order 9
    algebra = "invol_lattices"   # #50  cube length 78, order 13
    algebra = "moufang"          # #64  no good - cube length 48, order 6
    algebra = "loops"            # #36  cube length 36, order 8
    algebra = "tarski"           # #102 cube length 36, order 12
    algebra = "quasi_ordered"
    algebra = "posets"           # #86 cube length 36, order 9
    algebra = "dist_lattice_ord_semi"           # #86 cube length 36, order 9


    target_cube_length = 27
    order = 7

    t0 = time.time()
    gen_all_cubes(algebra, order, target_cube_length, threshold, mace4_exe)
    t1 = time.time()
    gen_cube_time = t1 - t0
    t2 = time.time()
    print(f"Done generating cubes for {algebra}, order {order}. Generating models...")
    models_count = run_all_cubes(algebra, order, target_cube_length, mace4_exe)
    t3 = time.time()
    runtime = t3 - t2
    collect_stat(algebra, order, target_cube_length, models_count, gen_cube_time, runtime)
    

#!/usr/bin/python3

"""
This is the entry point for running with parallel cubes and isomorphic cube removal

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
from datetime import datetime
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
print_models = "P0"  # P0 - don't output models, A1 - output models, P1 print models

cube_sequence_2 = [2, 4, 9, 16, 25, 36, 49, 64]   # for radius 2, 2, 3, 4, 5, 6, 7, ...
cube_sequence_1_2 = [2, 4, 6, 12, 20, 30, 42, 56, 72]   # for radius 1, 2, 2, 3, 4, 5, 6...
cube_sequence_1_2_2 = [3, 6, 10, 21, 36, 55, 78, 105, 136]
cube_sequence_2_2 = [2, 4, 8, 18, 32, 50, 72, 98, 128]
cube_sequence_2_2_2 = [3, 6, 12, 27, 48, 75, 108, 147, 215]
cube_sequence_1_1_2_2 = [4, 8, 12, 24, 40, 60, 84, 112, 144]   # for radius 1, 2, 2, 3, 4, 5, 6...
cube_sequence_1_2_2_2 = [4, 8, 14, 30, 52, 80, 114, 154, 200]
cube_sequence_2_2_2_2 = [4, 8, 16, 36, 64, 100, 144, 196, 256]
cube_sequence_1_2_2_2_2 = [5, 10, 18, 39, 68, 105, 150, 203, 264]

id_permuation = []
permutations2 = [[1, 0]]
permutations3 = permutations2 + [[0,2,1],[1,2,0],[2,1,0],[2,0,1]]
permutations4 =  analyzer.shorten_permutations(list(permutations(range(0, 4))))   # permute 0, 1, 2, 3 only
permutations5 = analyzer.shorten_permutations(list(permutations(range(0, 5))))
permutations6 = analyzer.shorten_permutations(list(permutations(range(0, 6))))
permutations7 = analyzer.shorten_permutations(list(permutations(range(0, 7))))
permutations8 = analyzer.shorten_permutations(list(permutations(range(0, 8))))
perm = {0: id_permuation, 1: permutations2, 2: permutations3, 3: permutations4, 4: permutations5, 5: permutations6, 6: permutations7, 7: permutations8}

# calculate radii
r_2 = {(x+1)**2: x for x in range(0, 8)} # 1 binary op {1: 0, 4: 1, 9: 2, 16: 3
r_2.update({2 : 1})
r_2_2 = {k*2: v for k, v in r_2.items()}   # 2 binary op
r_2_2_2 = {k*3: v for k, v in r_2.items()}   # 3 binary op
r_1_2   = {k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
r_1_2_2 = {2*k+v+1: v for k, v in r_2.items()}   # 1 unary op and 2 binary op
r_1_1_2_2 = {2*(k+v+1): v for k, v in r_2.items()}   # 2 unary op and 2 binary op
r_1_2_2_2   = {3*k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
r_2_2_2_2 = {k*4: v for k, v in r_2.items()}   # 4 binary op
r_1_2_2_2_2 = {k*4+v+1: v for k, v in r_2.items()}   # 1 unary op, 4 binary op
    
run_data = {'anti_monoid': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '123_antidomain_monoid', 'arities': [1, 2], 'radius': r_1_2, 'remove': 1},
            'assoc_dimonoid': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '160_associative_dimonoid', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'bck':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '3_bck', 'arities': [2], 'radius': r_2, 'remove': 0},
            'bck_lattice':   {'seq': cube_sequence_2_2_2_2, 'relations': [False, False, False, True], 
                         'input': '5_bck_lattices', 'arities': [2, 2, 2, 2], 'radius': r_2_2_2_2, 'remove': 0},
            'bounded_lattice': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '12_bounded_lattice', 'arities': [2, 2], 'radius': r_2_2, 'remove': 1},
            'brouwerian': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '15_brouwerian_sl', 'arities': [2, 2], 'radius': r_2_2, 'remove': 0},
            'clifford': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '17_clifford', 'arities': [1, 2], 'radius': r_1_2, 'remove': -1},
            'closable_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '125_closable_sp_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': 0},
            'comm_ord_monoid': {'seq': cube_sequence_2_2, 'relations': [False, True], 
                         'input': '23_comm_ord_monoid', 'arities': [2, 2], 'radius': r_2_2, 'remove': 0},
            'comm_ord_semi': {'seq': cube_sequence_2_2, 'relations': [False, True], 
                         'input': '24_comm_ord_semi', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'compl_mod_lattice': {'seq': cube_sequence_1_2_2, 'relations': [False, False, False], 
                         'input': '31_comp_modular_lattice', 'arities': [1, 2, 2], 'radius': r_1_2_2, 'remove': -1},
            'dimonoid': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '152_dimonoid', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'directoid':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '35_directoid', 'arities': [2], 'radius': r_2, 'remove': -1},
            'disemi': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '148_disemi', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'double_ward':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '144_double_ward_quasi', 'arities': [2], 'radius': r_2, 'remove': 0},
            'dist_lattice_ord_semi': {'seq': cube_sequence_2_2_2, 'relations': [False, False, False], 
                         'input': '36_dist_lattice_ord_semi', 'arities': [2, 2, 2], 'radius': r_2_2_2, 'remove': -1},
            'hilbert':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'hilbert', 'arities': [2], 'radius': r_2, 'remove': -1},
            'hoop': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '45_hoop', 'arities': [2, 2], 'radius': r_2_2, 'remove': 0},
            'idemp_semiring': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '110_idemp_semiring', 'arities': [2, 2], 'radius': r_2_2, 'remove': 1},
            'invol_quandle':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '127_involutory_quandle', 'arities': [2], 'radius': r_2, 'remove': -1},
            'invol_lattices': {'seq': cube_sequence_1_2_2, 'relations': [False, False, False], 
                         'input': '50_invol_lattices', 'arities': [1, 2, 2], 'radius': r_1_2_2, 'remove': -1},
            'inv_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '121_inv_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': -1},
            'left_closure_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '119_left_closure_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': -1},
            'left_disemi': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '146_left_disemi', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'monoid':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'monoids', 'arities': [2], 'radius': r_2, 'remove': 0},
            'mzeroid': {'seq': cube_sequence_1_2_2_2, 'relations': [False, False, False, True], 
                         'input': '58_m_zeroid', 'arities': [1, 2, 2, 2], 'radius': r_1_2_2_2, 'remove': 0},
            'mv_algebra': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '106_mv', 'arities': [1, 2], 'radius': r_1_2, 'remove': 1},
            'neardist': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '70_neardistr_lattice', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'normal_bands':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '71_normal_band', 'arities': [2], 'radius': r_2, 'remove': -1},
            'ockham': {'seq': cube_sequence_1_2_2, 'relations': [False, False, False], 
                         'input': '73_ockham', 'arities': [1, 2, 2], 'radius': r_1_2_2, 'remove': -1},
            'ord_algebra':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '74_order', 'arities': [2], 'radius': r_2, 'remove': 1},
            'ord_semilattice':   {'seq': cube_sequence_2_2, 'relations': [False, True], 
                         'input': '78_ord_semilattice', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'ortho':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '107_ortho', 'arities': [2], 'radius': r_2, 'remove': -1},
            'ortho_modular':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '108_ortho_modular', 'arities': [2], 'radius': r_2, 'remove': 0},
            #'quasi_holes': {'seq': cube_sequence_2_2, 'relations': [False, True], 
            #             'input': 'iqg4.n.2', 'arities': [2, 2], 'radius': r_2_2, 'remove': 1},
            'quasi_holes': {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'iqg4.n.2', 'arities': [2], 'radius': r_2, 'remove': -1},
            'quasi_impl':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '109_quasi_implication_algebra', 'arities': [2], 'radius': r_2, 'remove': -1},
            'quasi_mv': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '88_quasi_mv', 'arities': [1, 2], 'radius': r_1_2, 'remove': 1},
            'quasi_ordered': {'seq': cube_sequence_2, 'relations': [True], 
                         'input': 'quasi_ordered', 'arities': [2], 'radius': r_2, 'remove': -1},
            'quandles': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '87_quandles', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'right_hoop': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '93_right_hoop', 'arities': [2, 2], 'radius': r_2_2, 'remove': 0},
            'semi':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi', 'arities': [2], 'radius': r_2, 'remove': -1},
            'semi_varN12':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi_varN12', 'arities': [2], 'radius': r_2, 'remove': -1},
            'semi_varN12_idemp':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi_varN12_idemp', 'arities': [2], 'radius': r_2, 'remove': -1},
            'semi_varG2I2':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi_varG2I2', 'arities': [2], 'radius': r_2, 'remove': -1},
            'semi_varN':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi_varN', 'arities': [2], 'radius': r_2, 'remove': -1},
            'semizero':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semizero', 'arities': [2], 'radius': r_2, 'remove': 0},
            'skew_lattices': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '100_skew_lattices', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'sp_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '124_sp_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': 0},
            'loops':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '32_loop', 'arities': [2], 'radius': r_2, 'remove': 0},
            'm_zeriods':   {'seq': cube_sequence_1_2_2_2_2, 'relations': [False, False, False, False, True], 
                         'input': '58_m_zeroids', 'arities': [1, 2, 2, 2, 2], 'radius': r_1_2_2_2_2, 'remove': 0},
            'meadows':   {'seq': cube_sequence_1_1_2_2, 'relations': [False, False, False, False], 
                         'input': 'meadows', 'arities': [1, 1, 2, 2], 'radius': r_1_1_2_2, 'remove': 1},
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
    

def collect_cubes(algebra, cube_dir, prev_cube_length, cube_length, threshold, data_dir, num_threads):
    al = run_data[algebra]
    cmd = f"cat {cube_dir}_*/{cube_length}.out | grep '^cube' | sort --parallel={num_threads} -u | sed 's/[^ ]* //' > {data_dir}/cubes{cube_length}.out"
    subprocess.run(cmd, capture_output=False, text=True, check=False, shell=True)
    cube_file = f"{data_dir}/cubes{cube_length}.out"
    prev_file = f"{data_dir}/cubes_{order}_{prev_cube_length}.out"
    out_cube_file = f"{data_dir}/cubes_{order}_{cube_length}.out"
    
    radius = al['radius'][cube_length]
    all_permutations = perm[radius]
    if al['remove'] == 0:
        all_permutations = analyzer.remove0(all_permutations)
    elif al['remove'] == 1:
        all_permutations = analyzer.remove1(all_permutations)
    
    t1 = time.time()
    analyzer.gen_sequence(order, cube_length, radius, al['arities'], al['relations'], all_permutations, 
                          threshold, "utils/mace4/iso_cubes.py", prev_file, cube_file, out_cube_file, max_threads=num_threads)
    analyze_time = time.time() - t1
    return analyze_time
    

def gen_all_cubes(algebra, order, target_cube_length, threshold, mace4_exe, cubes_options, num_threads):
    """
    Args:
        algebra (str): name of the algebra
        order (int): order of the algebra
        target_cube_length (int): target cube length
        threshold (int):  use invariant when the number of models is at least this manany
        mace4_exe (str): path name of the mace4 executable
        num_threads (int): max number of threads to use
    """
    data_dir = get_data_dir(algebra, order)
    os.makedirs(data_dir, exist_ok=True)

    seq = run_data[algebra]['seq']
    cube_length = 0
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    propagated_models_count = 0
    for index, new_cube_length in enumerate(seq):
        new_cube_dir = get_working_dir(algebra, order, new_cube_length)
        if index == 0:
            cube_file = None
        else:
            cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{cube_length}.out"
        extend_cubes.extend_cubes(input_file, order, new_cube_length, cube_file, print_models, mace4_exe,
                                  new_cube_dir, num_threads, cubes_options, request_work_file, work_file)
        
        working_dir_prefix = get_working_dir(algebra, order, new_cube_length)    
        cmd = f'grep "Exiting with " {working_dir_prefix}_*/{new_cube_length}.out | grep model | utils/mace4/counter.py'
        # print(f'^^^^^^^^^^^^^^^^^^^^^^{cmd}^^^^^^^^^^^^^^^^^^^^^')
        sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
        propagated_models_count += int(sp.stdout)
        
        analyze_time = collect_cubes(algebra, new_cube_dir, cube_length, new_cube_length, threshold, data_dir, num_threads)        
        print(f"{algebra}, {order}, cube length={new_cube_length}, threshold={threshold}, analyze time {analyze_time}, #propagated models {propagated_models_count}", flush=True)
        if new_cube_length == target_cube_length:
            break
        cube_length = new_cube_length
    return propagated_models_count
    
    
def run_all_cubes(algebra, order, target_cube_length, mace4_exe, cubes_options):
    # print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")}, run all cubes...', flush=True)
    input_file = f"inputs/{run_data[algebra]['input']}.in"
    cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{target_cube_length}.out"
    working_dir_prefix = get_working_dir(algebra, order, target_cube_length)
    run_cubes.run_mace(mace4_exe, input_file, order, cube_file, print_models, cubes_options, working_dir_prefix, num_threads)
    
    cmd = f'grep "Exiting with " {working_dir_prefix}_*/mace.log | grep model | utils/mace4/counter.py'
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    return int(sp.stdout)

    
def collect_stat(algebra, order, target_cube_length, cube_options, threshold, models_count, gen_cube_time, runtime):
    data_dir = get_data_dir(algebra, order)
    out_cube_file = f"{data_dir}/cubes_{order}_{target_cube_length}.out"
    count = len(open(out_cube_file).readlines( ))
    print(f'"{algebra}", order={order}, invariant threshold={threshold}, options={cube_options}, cubes count={count}, {gen_cube_time}, #model={models_count}, model gen time={runtime}\n',
          flush=True)


__all__ = ["run_all_cubes", "gen_all_cubes", "collect_stat"]

if __name__ == "__main__":
    mace4_exe = "../bin/mace4"
    cubes_options = 1        # bit-0  set to 1 if use work-stealing
    threshold = 1000  # large number to disable invariants

    algebra = "quasi"
    algebra = "trigroup"
    algebra = "res_po_monoid"
    algebra = "moufang"          # #64  no good - cube length 48, order 6
    algebra = "dist_lattice_ord_semi"           # #86 cube length 36, order 9
    algebra = "order_algebras"   # #74  cube length  25, order 8
    algebra = "m_zeriods"        # #58 order 10 length 150
    algebra = "posets"           # #86 cube length 36, order 9
    algebra = "ord_semilattice"  # #78  cube length 72, order 9
    algebra = "quasi_holes"      # length 32 order 19 
    algebra = "tarski"           # #102 cube length 36, order 12
    algebra = "loops"            # #32  cube length 16, order 8
    algebra = "hilbert"
    algebra = "semizero"
    algebra = "semi_varN12"      # order 8
    algebra = "quandles"         # order 10
    algebra = "assoc_dimonoid"   # order = 6
    algebra = "quasi_impl"       # always half 
    algebra = "ortho_modular"    # no reduction
    algebra = "ortho"            # always half
    algebra = "ord_algebra"      # order 7/8
    algebra = "quasi_ordered"
    algebra = "semi_varG2I2"
    algebra = "ord_semilattice"  # no reduction
    algebra = "invol_lattices"   # #50  cube length 78, order 13, no reduction
    algebra = "comm_ord_monoid"
    algebra = "compl_mod_lattice"  #order 12  no reduction
    algebra = "inv_semi"         # #121 cube length 20 order 9
    algebra = "mzeroid"          # no reduction
    algebra = "bck"          # no reduction
    algebra = "anti_monoid"
    algebra = "semi_varN12_idemp"
    algebra = "skew_lattices"   # no reduction
    algebra = "mv_algebra"      # no reduction
    algebra = "hoop"            # no reduction
    algebra = "idemp_semiring"  # no reduction
    algebra = "left_closure_semi"
    algebra = "clifford"        # very little reduction
    algebra = "sp_semi"         # medium reduction, order 8
    algebra = "quasi_mv"        # medium reduction order 9
    algebra = "comm_ord_semi"   # no reduction
    algebra = "bck_lattice"   # no reduction order 8
    algebra = "ockham"   # no reduction order 9
    algebra = "dimonoid"         # order = 6
    algebra = "bounded_lattice"         # order = 10 no reduction
    algebra = "left_disemi"         # order = 6 some reduction
    algebra = "disemi"           # order = 6  
    algebra = "closable_semi"    # length 30 30% off
    algebra = "brouwerian"       # no reduction
    algebra = "neardist"       # no reduction
    algebra = "right_hoop"       # no reduction
    algebra = "meadows"         # no reduction
    algebra = "semi"
    algebra = "directoid"       # order 9, no reduction
    algebra = "invol_quandle"       # order 9, no reduction
    algebra = "normal_bands"

    target_cube_length = 25
    order = 8

    propagated_models_count = 0
    t0 = time.time()
    print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")} Generating all cubes...', flush=True)
    propagated_models_count += gen_all_cubes(algebra, order, target_cube_length, threshold, mace4_exe, cubes_options, num_threads)
    t1 = time.time()
    gen_cube_time = t1 - t0
    t2 = time.time()
    print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")} Done generating cubes for {algebra}, order {order}. Generating models...', flush=True)
    models_count = run_all_cubes(algebra, order, target_cube_length, mace4_exe, cubes_options)
    t3 = time.time()
    runtime = t3 - t2
    collect_stat(algebra, order, target_cube_length, cubes_options, threshold, models_count+propagated_models_count, gen_cube_time, runtime)
    print(f'Done {datetime.now().strftime("%d/%m/%Y %H:%M:%S")}')
    

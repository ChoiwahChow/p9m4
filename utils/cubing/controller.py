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


grep "Exiting with " semi_working_[0-9]/mace.log | grep model | utils/cubing/counter.py

"""


import sys
import os
from datetime import datetime
import time
import subprocess
import threading
import ast
import argparse
import shutil

import extend_cubes
import analyzer
import run_cubes
import run_isonaut
import isonaut

top_data_dir = "utils/cubing/working"


request_work_file = "request_work.txt"
work_file = "release_work.out"
# print_models = "A1"  # P0 - don't output models, A1 - output models, P1 print models

cube_sequence_2 = [2, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121]   # for radius 2, 2, 3, 4, 5, 6, 7, ...
cube_sequence_1_2 = [2, 4, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132]   # for radius 1, 2, 2, 3, 4, 5, 6...
cube_sequence_1_2_2 = [3, 6, 10, 21, 36, 55, 78, 105, 136]
cube_sequence_2_2 = [2, 4, 8, 18, 32, 50, 72, 98, 128]
cube_sequence_2_2_2 = [3, 6, 12, 27, 48, 75, 108, 147, 215]
cube_sequence_1_1_2_2 = [4, 8, 12, 24, 40, 60, 84, 112, 144]   # for radius 1, 2, 2, 3, 4, 5, 6...
cube_sequence_1_2_2_2 = [4, 8, 14, 30, 52, 80, 114, 154, 200]
cube_sequence_2_2_2_2 = [4, 8, 16, 36, 64, 100, 144, 196, 256]
cube_sequence_1_2_2_2_2 = [5, 10, 18, 39, 68, 105, 150, 203, 264]

run_data = {'inverse_semi' : {'seq': cube_sequence_1_2},
            'loops' : {'seq': cube_sequence_2},
            'semi' : {'seq': cube_sequence_2}} 

"""
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
                         'input': '31_comp_modular_lattice', 'arities': [1, 2, 2], 'radius': r_1_2_2, 'remove': 1},
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
            'monoidplus':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'monoid_12', 'arities': [2], 'radius': r_2, 'remove': 0},
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
            'quasi_holes_2_2': {'seq': cube_sequence_2_2, 'relations': [False, True], 
                         'input': 'sem_iqg4.n.2', 'arities': [2, 2], 'radius': r_2_2, 'remove': 1},
            'quasi_holes_2': {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'iqg4.n.2', 'arities': [2], 'radius': r_2, 'remove': -1},
            'quasi_impl':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': '109_quasi_implication_algebra', 'arities': [2], 'radius': r_2, 'remove': -1},
            'quasi_mv': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '88_quasi_mv', 'arities': [1, 2], 'radius': r_1_2, 'remove': 1},
            'quasi_ordered': {'seq': cube_sequence_2, 'relations': [True], 
                         'input': '89_quasi_ordered', 'arities': [2], 'radius': r_2, 'remove': -1},
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
                         'input': '94_semi_w_0', 'arities': [2], 'radius': r_2, 'remove': 0},
            'skew_lattices': {'seq': cube_sequence_2_2, 'relations': [False, False], 
                         'input': '100_skew_lattices', 'arities': [2, 2], 'radius': r_2_2, 'remove': -1},
            'sp_semi': {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': '124_sp_semi', 'arities': [1, 2], 'radius': r_1_2, 'remove': 0},
            'iploops':   {'seq': cube_sequence_1_2, 'relations': [False, False], 
                         'input': 'iploops', 'arities': [1, 2], 'radius': r_1_2, 'remove': 0},
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
                         'input': '102_tarski', 'arities': [2], 'radius': r_2, 'remove': -1},
            'trigroup': {'seq': cube_sequence_1_2_2_2, 'relations': [False, False, False, False], 
                         'input': '142_trigroup', 'arities': [1, 2, 2, 2], 'radius': r_1_2_2_2, 'remove': 0}}
"""


def get_data_dir(algebra, order):
    return f"{top_data_dir}/{algebra}{order}"


def get_working_dir(algebra, order, cube_length):
    """ compose working dir
    """
    return f"{algebra}_working_{order}_{cube_length}"

def get_working_dir_prefix(algebra, order):
    return f"{algebra}_working_{order}" 

def collect_cubes(order, cube_dir, cube_length, data_dir, num_threads):
    """ collect the cubes generated, and remove isomorphic cubes
    """
    # get rid of leading "cube "
    # cmd = f"cat {cube_dir}_*/{cube_length}.out | grep '^cube' | sort --parallel={num_threads} -u | sed 's/[^ ]* //' > {data_dir}/cubes{cube_length}.out"
    cmd = f"cat {cube_dir}_*/{cube_length}.out | grep '^cube' | sed 's/[^ ]* //' > {data_dir}/cubes{cube_length}.out"
    subprocess.run(cmd, capture_output=False, text=True, check=False, shell=True)

    cube_file = f"{data_dir}/cubes{cube_length}.out"
    out_cube_file = f"{data_dir}/cubes_{order}_{cube_length}.out"
    
    t1 = time.time()
    analyzer.gen_sequence (cube_file, out_cube_file, max_threads=num_threads)
    analyze_time = time.time() - t1
    return analyze_time
    

def gen_all_cubes(mace4_args, target_cube_length, num_threads):
    """
    Args:
        algebra (str): name of the algebra
        order (int): order of the algebra
        target_cube_length (int): target cube length
        mace4_exe (str): path name of the mace4 executable
        cubes_options (int): bit vector lsb 1 - work stealing, 2 - cube has num cells filled at beginning
        num_threads (int): max number of threads to use
    """
    algebra = mace4_args['algebra']
    order = mace4_args['order']
    print_model = mace4_args['print_model']
    mace4_exe = mace4_args['mace4_exe']
    input_file = mace4_args['input_file']
    cubes_options = mace4_args['cubes_options']

    data_dir = get_data_dir(algebra, order)
    os.makedirs(data_dir, exist_ok=True)

    seq = run_data[algebra]['seq']
    cube_length = 0
    propagated_models_count = 0
    for index, new_cube_length in enumerate(seq):
        new_cube_dir = get_working_dir(algebra, order, new_cube_length)
        os.makedirs(f"{new_cube_dir}_models", exist_ok=True)
        if index == 0:
            cube_file = None
        else:
            cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{cube_length}.out"
        extend_cubes.extend_cubes(mace4_args, new_cube_length, cube_file, 
                                  new_cube_dir, num_threads, request_work_file, work_file)
        
        working_dir_prefix = get_working_dir(algebra, order, new_cube_length)    
        cmd = f'grep "Exiting with " {working_dir_prefix}_*/{new_cube_length}.out | grep model | utils/cubing/counter.py'
        # print(f'^^^^^^^^^^^^^^^^^^^^^^{cmd}^^^^^^^^^^^^^^^^^^^^^')
        sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
        propagated_models_count += int(sp.stdout)

        # filter non-iso for propagated models, if any
        if int(sp.stdout) > 0:
            run_isonaut.run_isonaut( working_dir_prefix, num_threads, mace4_args['output_file'], 1, num_threads )

        analyze_time = collect_cubes(order, new_cube_dir, new_cube_length, data_dir, num_threads)        
        print(f"{algebra}, {order}, cube length={new_cube_length}, analyze time {analyze_time}, #propagated models {propagated_models_count}", flush=True)
        
        if new_cube_length >= target_cube_length:
            break
        cube_length = new_cube_length
    return propagated_models_count
    
    
def run_all_cubes(mace4_args, target_cube_length, num_threads):
    # print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")}, run all cubes...', flush=True)
    # input_file = f"inputs/{run_data[algebra]['input']}.in"
    order = mace4_args['order']
    algebra = mace4_args['algebra']
    out_filename = mace4_args['output_file']
    cubes_options = mace4_args['cubes_options']

    cube_file = f"{top_data_dir}/{algebra}{order}/cubes_{order}_{target_cube_length}.out"
    working_dir_prefix = get_working_dir(algebra, order, target_cube_length)
    run_cubes.run_mace(mace4_args, cube_file, working_dir_prefix, num_threads)
    
    cmd = f'grep "Exiting with " {working_dir_prefix}_*/mace.log | grep model | utils/mace4/counter.py'
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    model_count = int(sp.stdout)

    # iso-filtering
    run_isonaut.run_isonaut( working_dir_prefix, num_threads, mace4_args['output_file'], 1, num_threads )

    # final step of iso-filtering
    models_dir_prefix = get_working_dir_prefix(algebra, order)
    model_files = list()
    cube_seq = run_data[algebra]['seq']
    for x in cube_seq:
        if x > target_cube_length:
            break;
        for y in range(0, num_threads):
            model_dir = f"{models_dir_prefix}_{x}_{y}_models"
            if os.path.isdir(model_dir) and os.path.isfile(f"{model_dir}/{out_filename}"):
                model_files.append(f"{model_dir}/{out_filename}")
    _, num_non_iso = isonaut.process_files(model_files, f"outputs/{out_filename}", 0) 

    return model_count, num_non_iso


def collect_iso_filter_time():
    """ Extract cpu and wall clock times from
        Done CPU time = 1.09120982 s.  Wall-clock time = 1.1853587627410889 seconds
    """

    cmd = "grep 'CPU time' ../../*_working_*[0-9]/isonaut.log | gawk -F' ' '{ sum += $5 } END{ print sum, NR }'"
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    cpu_time = float(sp.stdout)
    
    cmd = "grep 'CPU time' ../../*_working_*[0-9]/isonaut.log | gawk -F' ' '{ sum += $10 } END{ print sum, NR }'"
    sp = subprocess.run(cmd, capture_output=True, text=True, check=False, shell=True)
    wall_time = float(sp.stdout)

    return cpu_time, wall_time

 
def collect_stat(mace4_args, target_cube_length, cube_options, models_count, num_non_iso,
                 gen_cube_time, runtime, iso_cpu_time, iso_wall_time):
    order = mace4_args['order']
    algebra = mace4_args["algebra"]
    data_dir = get_data_dir(algebra, order)
    out_cube_file = f"{data_dir}/cubes_{order}_{target_cube_length}.out"
    count = len(open(out_cube_file).readlines( ))
    print(f'\nSummary: algebra: {algebra}, order={order}, ptions={cube_options}, cubes count={count}')
    print(f'Gen cube time: {gen_cube_time}, #model={models_count}, model gen time={runtime}')
    print(f'Isofilter (hashing only) : cpu time={iso_cpu_time}, wall-clock time={iso_wall_time}, #Non-iso models={num_non_iso}\n', flush=True)


__all__ = ["run_all_cubes", "gen_all_cubes", "collect_stat"]

if __name__ == "__main__":
    mace4_exe = "./build/mace4"
    cubes_options = 1      # bit-0  set to 1 if use work-stealing
    algebra = "semi"
    print_model = "A-1"
    print_canonical = 0  # print the canonical graph for the final output models
    num_threads = 1

    parser = argparse.ArgumentParser(
        description='parallel processing for finite model generation.')
    parser.add_argument('input_file', nargs='+', type=str, default=None,
                        help='mace4 input files.')
    parser.add_argument('-o', dest='output_file', type=str, default="models.out", help='models output file path')
    parser.add_argument('-l', dest='target_cube_length', type=int, default=2, help='target cube length')
    parser.add_argument('-n', dest='order', type=int, default=3, help='order of algebra')
    parser.add_argument('-a', dest='algebra', type=str, default=algebra, help='short name of algebra, no space')
    parser.add_argument('-p', dest='print_model', type=str, default=print_model, help='mace4 print model option')
    parser.add_argument('-w', dest='print_canonical', type=int, default=print_canonical,
                        help='output the canonical graph')
    parser.add_argument('-t', dest='num_threads', type=int, default=num_threads)
    args = parser.parse_args()

    if args.print_model.startswith("A"):
        max_cache = args.print_model.replace("A", "W")
    else:
        max_cache = "W-1"
    mace4_args = { 'mace4_exe': mace4_exe, 'cubes_options': cubes_options,
                   'hook': '../utils/cubing/isonaut.sh', 'max_cache': max_cache,
                   'algebra': args.algebra, 'order': args.order, 
                   'input_file': args.input_file[0], 'output_file': args.output_file,
                   'print_model': args.print_model, 'print_canonical': args.print_canonical }
    target_cube_length = args.target_cube_length
    num_threads = args.num_threads

    propagated_models_count = 0
    t0 = time.time()
    print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")} Generating all cubes to target length {target_cube_length} ...', flush=True)
    propagated_models_count += gen_all_cubes(mace4_args, target_cube_length, num_threads)
    t1 = time.time()
    gen_cube_time = t1 - t0
    t2 = time.time()

    # Model generation
    print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")} Done generating cubes for {mace4_args["algebra"]}, order {mace4_args["order"]}. Generating models...', flush=True)
    models_count, num_non_iso = run_all_cubes(mace4_args, target_cube_length, num_threads)

    t3 = time.time()
    runtime = t3 - t2

    (iso_cpu_time, iso_wall_time) = collect_iso_filter_time()

    collect_stat(mace4_args, target_cube_length, cubes_options, models_count+propagated_models_count, num_non_iso, gen_cube_time, runtime, iso_cpu_time, iso_wall_time)
    print(f'Done {datetime.now().strftime("%d/%m/%Y %H:%M:%S")}')
    

#!/usr/bin/python3
"""
Remove isomorphic cubes for multiple tables



To generate cubes:
date; build/mace4 -n7 -N7 -m-1 -A1 -C16 -O3 -f inputs/semi.in > working/16.out; date
The models generated in this step must be kept

Remove duplicate cubes
grep "^cube" working/2.out | sort | uniq | sed 's/[^ ]* //' > working/semi7/cubes2.out


The cubes are of the format (the first number is num cells filled, -1 if not calculated)
4 0 0 0 0
4 0 0 1 1
9 0 0 2 2
4 0 1 0 0




Examples of ordered cells for cube length of 25, the first element in the tuple is the function (represented by a number), the second is the cell coordinates.
>>> arities = [1, 2]
>>> gen_func_cells(cube_length, arities)

>>> arities = [2, 2, 2]
>>> gen_func_cells(cube_length, arities)
[[((0, (0, 0)), 0), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], [((0, (0, 0)), 1), ((1, (0, 0)), 0), ((2, (0, 0)), 0)]]
"""

import sys
import subprocess
import threading
import time
import copy
import json
from itertools import permutations

import invariants
import iso_cubes

min_parallel_blocks = 3


def thread_available(thread_count, thread_slots):
    for x in range(thread_count):
        if thread_slots[x] == 0:
            return x
    return -1


def all_done(thread_slots):
    for x in range(len(thread_slots)):
        if thread_slots[x] != 0:
            return False
    return True

"""
def run_process(id, slot_id, thread_slots, order, cube_length, input_file, cube, print_models, mace4, working_dir):
    working_dir = f"{working_dir}_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    
    if cube is not None:
        with (open(f"{working_dir}/cube.config", "w")) as fp:
            for x in cube:
                fp.write(f"{x}\n")

    # print(f"************************************ {cube}")
    subprocess.run(f"cd {working_dir}; {mace4} -n{order} -N{order} -m-1 -{print_models} -C{cube_length} -O3 -f {input_file} >> {cube_length}.out 2>>mace.out", 
                    capture_output=False, text=True, check=False, shell=True)      # ; mv models.out {id}.out",
    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0
"""


def permute(s, a):
	if len(s) > a:
		return s[a]
	else:
		return a


def find_cube_radius(cube):
	r = 0
	for x in cube:
		for y in x[0][1]:
			if y > r:
				r = y
	return r


def find_permutation_mdn(p):
	return len(p) - 1


def simplify_permutation(p):
	""" simplify, e.g. [0, 2, 1, 3, 4] to [0, 2, 1] because it does not move 3 or 4.
	"""
	l = len(p) - 1
	p = list(p)
	while l >= 0 and p[l] == l:
		p.pop()
		l -= 1
	return p


def shorten_permutations(ps):
    el = len(ps[0]) - 1
    if el > 2:
        ps = [x for x in ps if x[el] != el]
    x = [simplify_permutation(p) for p in ps]
    return [y for y in x if y]


def isomorphic_cube(cube, permutation, is_relation):
	"""
		cube (List[List[int, List[int]], int]): a cube, sorted by table, then by first coordinate, then second coordinate
		permutation(List[int]): a permutation
	"""
	eq_cube = list()
	for x in cube:
		cell = tuple(permute(permutation, y) for y in x[0][1])
		if is_relation[int(x[0][0])]:
			term = tuple([(x[0][0], cell), x[1]])
		else:
			term = tuple([(x[0][0], cell), permute(permutation, x[1])])
		eq_cube.append(term)
	return tuple(sorted(eq_cube))		
					

def has_iso(x, is_relation, all_permutations, non_iso_sorted):
	"""  check isomorphism against all given permutations
	Args:
		x (List[List[int, List[int]], int]): a cube, sorted by table, then by first coordinate, then second coordinate
		all_permutations (List[list[int]]): a list of permutations
		r (int):     radius
		non_iso_sorted (dict): dictionary of non-isomorphic cubes, sorted by table, then by first coordinate, then second coordinate
	"""
	if x in non_iso_sorted:
		return True
	for p in all_permutations:
		iso_cube = isomorphic_cube(x, p, is_relation)
		if iso_cube in non_iso_sorted:
			return True
	return False


def remove_isomorphic(cubes, is_relation, all_permutations):
	""" removes all cubes that have isomorphic cubes already seen
	Args:
		cubes: list of cubes
		is_relation (List[bool]): the operation is a relation
		all_permutations (List[list[int]]): a list of permutations
	"""
	non_iso_sorted = dict()
	non_iso_unsorted = list()
	for x in cubes:
		y = tuple(sorted(x))   # sorted by table, then by first coordinate, then second coordinate
		if not has_iso(y, is_relation, all_permutations, non_iso_sorted):
			non_iso_unsorted.append(tuple(x))
			non_iso_sorted[y] = 1
	return tuple(non_iso_unsorted)


def run_process(id, slot_id, thread_slots, cubes, is_relation, all_permutations, seq, iso_cubes_exec):
    params = {'cubes': cubes, 'is_relation': is_relation, 'all_permutations': all_permutations}
    params_json = json.dumps(params)
    
    cp = subprocess.run(iso_cubes_exec, input=params_json.encode('utf-8'), stdout=subprocess.PIPE, shell=True)      # ; mv models.out {id}.out",
    out_json = json.loads(cp.stdout.decode('utf-8'))
    non_iso_cubes = [[tuple([tuple([y[0][0], tuple(y[0][1])]), y[1]]) for y in x] for x in out_json]
    # if len(cubes) != len(non_iso_cubes):
    #     print(f"******{len(cubes}********cut down to********{len(non_iso_cubes)}")
    seq.extend(non_iso_cubes)
    thread_slots[slot_id] = 0


def run_process_multi(id, slot_id, thread_slots, blocks, is_relation, all_permutations, seq, iso_cubes_exec_multi):
    hash_values = [x[1] for x in blocks]
    params = {'blocks': hash_values, 'is_relation': is_relation, 'all_permutations': all_permutations}
    params_json = json.dumps(params)
    # print(f"debug run_process_multi ^^^^^^^^^^^^^^^{params_json}^^^^^^^^^^^^")
    
    cp = subprocess.run(iso_cubes_exec_multi, input=params_json.encode('utf-8'), stdout=subprocess.PIPE, shell=True)      # ; mv models.out {id}.out",
    # print(f"^^^^^^^^^^^^^^^{cp.stdout}^^^^^^^^^^^^")
    out_json = json.loads(cp.stdout.decode('utf-8'))
    # non_iso_cubes = [[tuple([tuple([y[0][0], tuple(y[0][1])]), y[1]]) for y in x] for x in out_json]
    non_iso_cubes = [tuple([[tuple([tuple([y[0][0], tuple(y[0][1])]), y[1]]) for y in x[0]], x[1]]) for x in out_json]
    # if len(cubes) != len(non_iso_cubes):
    #     print(f"******{len(cubes}********cut down to********{len(non_iso_cubes)}")
    seq.extend(non_iso_cubes)
    thread_slots[slot_id] = 0


def is_in_prev(all_cubes, x):
	for prefix in all_cubes:
		if x.startswith(prefix):
			return True
	return False
        
        
def gen_sequence_multi(n, cube_length, radius, arities, is_relation, all_permutations, inv_threshold, iso_cubes_exec_multi,
                 prev_cubes_filepath, in_cubes_filepath, out_cubes_filepath, max_threads=8):
    """
        A cube is represented by a list of (cell term, value), and in addition, there is a number of cells filled for each cube.
        e.g. cubes:  [[[((0, (0, 0)), 0), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], 8], [[((0, (0, 0)), 1), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], 7]]
    Args:
        n (int): order of the algebra
        cube_length (int): length of the search sequence
        radius (int): radius of the cube
        arities (List[int]): arity of each function
        is_relation (List[bool]): True if if the function symbol is a relation
        all_permutations:
        inv_threshold (int): use invariants if there are at least inv_threshold models
        iso_cubes_exec (str): path of python secript to filter isomoprhic cubes
        prev_cubes_filepath (str): file path of a file containing all cubes of shorter length to build on
        in_cubes_filepath (str): file path of the file containing the cubes generated by Mace4
        out_cubes_filepath (str): file for output cubes of length "cube_length"
        all_permutations (List[List[int, int]]): list of permutations
    """    
    print(f"debug gen_sequence_multi Number of permutations {len(all_permutations)}")
    # prev_str = read_cubes_file(prev_cubes_filepath)    
    full_cubes_str = read_cubes_file(in_cubes_filepath)
    
    ordered_cells = gen_func_cells(cube_length, arities)
    
    # print(f"***** {cube_length} {ordered_cells}")
    all_cubes = list()
    for cube_str in full_cubes_str:
        cell_values = [int(x) for x in cube_str.split(" ")]
        cube = ([(x, cell_values[index+1]) for index, x in enumerate(ordered_cells)], cell_values[0])
        all_cubes.append(cube)
    
    print(f"debug gen_sequence_multi, starting number of cubes: {len(full_cubes_str)}, {len(ordered_cells)}", flush=True)
    if len(all_cubes) > 2000000:
        all_permutations = []
    if all_permutations and (len(all_cubes) > inv_threshold or len(all_permutations) > 5000):
        buckets = invariants.calc_invariant_vec(all_cubes, radius, arities, is_relation)
        all_blocks = sorted(buckets.items(), key=lambda item: len(item[1]), reverse=True)   # list of [key, value]
        thread_slots = [0] * max_threads
        seq = list()
        while all_blocks:
            num = len(buckets)/max_threads
            if num > 1000:
                num = 500
            elif num > 100:
                num = 100
            elif num > 10:
                num = 10
            elif num >= 2:
                num = 2
            else:
                num = 1
            blocks = all_blocks[:num]
            all_blocks = all_blocks[num:]
            
            if len(blocks) == 1 and len(blocks[0]) == 1:
                a_seq = iso_cubes.remove_isomorphic_cubes(blocks[0], is_relation, all_permutations)
                seq.extend(a_seq)
            else:
                slot_id = thread_available(max_threads, thread_slots)
                while slot_id < 0:
                    time.sleep(0.05)
                    slot_id = thread_available(max_threads, thread_slots)
                thread_slots[slot_id] = threading.Thread(target=run_process_multi,
                                                         args=(id, slot_id, thread_slots, blocks, is_relation, all_permutations, seq, iso_cubes_exec_multi))
                thread_slots[slot_id].start()
        while not all_done(thread_slots):
            time.sleep(0.1)
    else:
        seq = iso_cubes.remove_isomorphic_cubes(all_cubes, is_relation, all_permutations)
    
    seq = sorted(seq, key=lambda item: item[1], reverse=True)
    print(f"Debug************* {seq[0][1]} {seq[-1][1]}")
    with (open(out_cubes_filepath, "w")) as fp:
        for cube in seq:
            for term in cube[0]:
                fp.write(f"{term[1]} ")
            fp.write("\n")
    print(f"debug gen_sequence_multi, final number of cubes: {len(seq)}, {len(ordered_cells)}")
        
		
		
def gen_sequence(n, cube_length, radius, arities, is_relation, all_permutations, inv_threshold, iso_cubes_exec,
				 prev_cubes_filepath, in_cubes_filepath, out_cubes_filepath, max_threads=8):
    """
    	n (int): order of the algebra
    	cube_length (int): length of the search sequence
    	radius (int): radius of the cube
    	arities (List[int]): arity of each function
    	is_relation (List[bool]): True if if the function symbol is a relation
    	all_permutations:
    	inv_threshold (int): use invariants if there are at least inv_threshold models
    	iso_cubes_exec (str): path of python secript to filter isomoprhic cubes
    	prev_cubes_filepath (str): file path of a file containing all cubes of shorter length to build on
    	in_cubes_filepath (str): file path of the file containing the cubes generated by Mace4
    	out_cubes_filepath (str): file for output cubes of length "cube_length"
    	all_permutations (List[List[int, int]]): list of permutations
    """	
    print(f"Number of permutations {len(all_permutations)}")
    # prev_str = read_cubes_file(prev_cubes_filepath)	
    full_cubes_str = read_cubes_file(in_cubes_filepath)
    
    ordered_cells = gen_func_cells(cube_length, arities)
    
    # print(f"***** {cube_length} {ordered_cells}")
    all_cubes = list()
    for cube_str in full_cubes_str:
        cell_values = [int(x) for x in cube_str.split(" ")]
        #cube = list()
        #for index, x in enumerate(ordered_cells):
        #    cube.append((x, cell_values[index+1]))
        cube = ([(x, cell_values[index+1]) for index, x in enumerate(ordered_cells)], cell_values[0])
        all_cubes.append(cube)
    # print(f"{all_cubes}")
    
    print(f"debug gen_sequence, starting number of cubes: {len(full_cubes_str)}, {len(ordered_cells)}", flush=True)
    if len(all_cubes) > inv_threshold:
        blocks = invariants.calc_invariant_vec(all_cubes, radius, arities, is_relation)
        print(f"debug gen_sequence parallel invariants^^^^^^^^^^{len(all_cubes)}, blocks {len(blocks)}^^^^^^^^^^^^^^^^^^called")
        thread_slots = [0] * max_threads
        seq = list()
        for key, same_inv_cubes in sorted(blocks.items(), key=lambda item: item[1], reverse=True):
            # for key, same_inv_cubes in blocks.items():
            #if len(same_inv_cubes) == 1:
            #    seq.append(tuple(same_inv_cubes[0]))
            if len(same_inv_cubes) < min_parallel_blocks:
                a_seq = iso_cubes.remove_isomorphic_cubes(same_inv_cubes, is_relation, all_permutations)
                seq.extend(a_seq)
            else:
                slot_id = thread_available(max_threads, thread_slots)
                while slot_id < 0:
                    time.sleep(0.05)
                    slot_id = thread_available(max_threads, thread_slots)
                thread_slots[slot_id] = threading.Thread(target=run_process,
                										 args=(id, slot_id, thread_slots, same_inv_cubes, is_relation, all_permutations, seq, iso_cubes_exec))
                thread_slots[slot_id].start()
        while not all_done(thread_slots):
            time.sleep(0.1)
    else:
        seq = iso_cubes.remove_isomorphic_cubes(all_cubes, is_relation, all_permutations)
    
    with (open(out_cubes_filepath, "w")) as fp:
        for cube in seq:
            for term in cube:
                fp.write(f"{term[1]} ")
            fp.write("\n")
    print(f"debug gen_sequence, final number of cubes: {len(seq)}, {len(ordered_cells)}")


def read_cubes_file(file_path):
	with (open(file_path)) as fp:
		p = fp.read().splitlines()
	# print(p)
	return p


def write_purged_cubes(file_path, cubes):
	with (open(file_path, "w")) as fp:
		json.dump(cubes, fp)


def print_short(cube):
	y = [str(x[1]) for x in cube]
	print("-".join(y))

# The following are unused. Keep for a short time
def transpose(a, b, x):
	if x == a:
		return b
	elif x == b:
		return a
	else:
		return x
	
	
def has_term(y, t):
	for t2 in y:
		if t == t2:
			return True
	return False


def equal_terms(x, y):
	if len(x) != len(y):
		return False
	for t in x:
		if not has_term(y, t):
			return False
	return True


def remove0(ps):
	return [x for x in ps if x[0] == 0]


def remove1(ps):
    return [x for x in ps if x[0] == 0 and x[1] == 1]


def gen_func_cells(cube_length, arities):
	r = 0
	cells = list()
	while len(cells) < cube_length:
		# diagonal cells first
		for index, a in enumerate(arities):
			new_cell = list()
			for x in range(0, a):
				new_cell.append(r)
			cells.append((index, tuple(new_cell)))
			if len(cells) >= cube_length:
				break
		if len(cells) >= cube_length:
			break
		for index, a in enumerate(arities):
			if a == 1:
				continue
			for x in range(0, r):         # a is assumed to be 2
				cells.append((index, tuple([x, r])))
				if len(cells) >= cube_length:
					break
				cells.append((index, tuple([r, x])))
				if len(cells) >= cube_length:
					break
		r = r + 1
	return tuple(cells)
			

__all__ =["gen_sequence", "gen_sequence_multi", "shorten_permutations", "remove0", "remove1", "remove_notN"]


if __name__ == "__main__":
    inv_threshold = 10000
    iso_cubes_exec = './iso_cubes.py'
    iso_cubes_exec_multi = './iso_cubes_multi.py'
    permutations2 = [[1, 0]]
    permutations3 = permutations2 + [[0,2,1],[1,2,0],[2,1,0],[2,0,1]]
    # permutations = [[0, 2, 1]]
    permutations4 =  shorten_permutations(list(permutations(range(0, 4))))   # permute 0, 1, 2, 3 only
    permutations5 = shorten_permutations(list(permutations(range(0, 5))))
    permutations6 = shorten_permutations(list(permutations(range(0, 6))))
    permutations7 = shorten_permutations(list(permutations(range(0, 7))))
    perm = {1: permutations2, 2: permutations3, 3: permutations4, 4: permutations5, 5: permutations6, 6: permutations7}
    
    # calculate radii
    r_2 = {(x+1)**2: x for x in range(0, 6)} # 1 binary op {1: 0, 4: 1, 9: 2, 16: 3
    r_2.update({2 : 1})
    r_2_2 = {k*2: v for k, v in r_2.items()}   # 2 binary op
    r_2_2_2 = {k*3: v for k, v in r_2.items()}   # 3 binary op
    r_1_2   = {k+v+1: v for k, v in r_2.items()}   # 1 unary op and 1 binary op
    order = 8
    prev_cube_length = 32
    cube_length = 50
    
    algebra = "iploop"
    algebra = "disemi"
    algebra = "inv_semi"
    algebra = "semi"
    algebra = "quandles"
    
    arities = {"disemi": [2, 2], "inv_semi": [1, 2], "quandles": [2, 2], "semi": [2]}
    radii = {"disemi": r_2_2, "inv_semi": r_1_2, "quandles": r_2_2, "semi": r_2}
    is_relation = {"disemi": [False, False], "inv_semi": [False, False], "quandles": [False, False], "semi": [False]}
    
    radius = radii[algebra][cube_length]
    if algebra in ["hilbert", "semizero", "loops", "iploop"]:
    	all_permutations = remove0(perm[radius])
    else:
    	all_permutations = perm[radius]
    
    cube_file = f"working/{algebra}{order}/cubes{cube_length}.out"
    prev_file = f"working/{algebra}{order}/cubes_2_{order}_{prev_cube_length}.out"
    out_cube_file = f"working/{algebra}{order}/cubes_2_{order}_{cube_length}.out"
    
    print(f"order: {order}, cube_length: {cube_length}, previous file: {prev_file}")
    # print(f"permutations: {all_permutations}")
    print(f"Number of permutations {len(all_permutations)}")
    gen_sequence(order, cube_length, radius, arities[algebra], is_relation[algebra], all_permutations, 
                 inv_threshhold, iso_cubes_exec, prev_file, cube_file, out_cube_file)




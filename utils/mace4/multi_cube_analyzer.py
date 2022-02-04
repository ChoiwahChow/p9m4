#!/usr/bin/python3
"""
Remove isomorphic cubes for multiple tables



To generate cubes:
date; build/mace4 -n7 -N7 -m-1 -A1 -C16 -O3 -f inputs/semi.in > 16.out; date
The models generated in this step must be kept

Remove duplicate cubes
grep "^cube" 2.out | sort | uniq | sed 's/[^ ]* //' > cubes2.out

Examples of ordered cells for cube length of 25
>>> arities = [1, 2]
>>> gen_func_cells(cube_length, arities)
((0, (0,)), (1, (0, 0)), (0, (1,)), (1, (1, 1)), (1, (0, 1)), (1, (1, 0)), (0, (2,)), (1, (2, 2)), (1, (0, 2)), (1, (1, 2)), (1, (2, 0)), (1, (2, 1)), (0, (3,)), (1, (3, 3)), (1, (0, 3)), (1, (1, 3)), (1, (2, 3)), (1, (3, 0)), (1, (3, 1)), (1, (3, 2)), (0, (4,)), (1, (4, 4)), (1, (0, 4)), (1, (1, 4)), (1, (2, 4)), (1, (3, 4)))
>>> arities = [2, 2]
>>> gen_func_cells(cube_length, arities)
((0, (0, 0)), (1, (0, 0)), (0, (1, 1)), (1, (1, 1)), (0, (0, 1)), (0, (1, 0)), (1, (0, 1)), (1, (1, 0)), (0, (2, 2)), (1, (2, 2)), (0, (0, 2)), (0, (1, 2)), (0, (2, 0)), (0, (2, 1)), (1, (0, 2)), (1, (1, 2)), (1, (2, 0)), (1, (2, 1)), (0, (3, 3)), (1, (3, 3)), (0, (0, 3)), (0, (1, 3)), (0, (2, 3)), (0, (3, 0)), (0, (3, 1)), (0, (3, 2)))

"""

import sys
import copy
import json
from itertools import permutations


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


def find_cube_mdn(cube):
	m = 0
	for x in cube:
		if x[1] > m:
			m = x[1]
		for y in x[0][1]:
			if y > m:
				m = y
	return m


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
	x = [simplify_permutation(p) for p in ps]
	return [y for y in x if y]


def ordering_cells_1_2(length):
	""" ordering: diagonal first, then minimum row, then minimum column,
	  then next minimum row, then next minimum column ...
	  e.g. (0), (0, 0), (1), (1, 1), (0, 1), (1, 0), (2), (2, 2), (0, 2), (2, 0), (1, 2), (2, 1), (3) (3, 3)...
	Args:
		length (int): maximum depth of the search search, note count starts from 0,
						 so a max depth of 2 is 0 and 1.
	"""
	seq = list()
	d = 0
	while len(seq) < length:
		seq.append((d,))
		seq.append((d, d))
		if len(seq) >= length:
			return tuple(seq)
		for x in range(0, d):
			seq.append((x, d))
			if len(seq) >= length:
				return tuple(seq)
			seq.append((d, x))
			if len(seq) >= length:
				return tuple(seq)
		d += 1
	return tuple(seq)


def equivalent_cube(cube, permutation, is_relation):
	"""
		cube (List[List[int, List[int]], int]): a cube
		permutation(List[int]): a permutation
	"""
	eq_cube = list()
	for x in cube:
		cell = tuple(permute(permutation, y) for y in x[0][1])
		if is_relation[int(x[0][0])]:
			term = tuple([cell, x[1]])
		else:
			term = tuple([cell, permute(permutation, x[1])])
		eq_cube.append(term)
	return tuple(sorted(eq_cube))		
					

def has_iso(x, is_relation, all_permutations, mdn, r, non_iso):
	"""  check isomorphism against all given permutations
		x (List[List[int, List[int]], int]): a cube
		all_permutations (List[list[int]]): a list of permutations
		mdn (int):   mdn
		r (int):     radius
		non_iso (dict): dictionary of non-isomorphic cubes
	"""
	if x in non_iso and non_iso[x][0] == mdn:
		return True
	for p in all_permutations:
		mdn_p = find_permutation_mdn(p)
		if not is_relation and mdn_p > r:   # mdn of permutation must not be greater than radius, unless it is a relation
			continue
		eq_cube = equivalent_cube(x, p, is_relation)
		if not eq_cube in non_iso:
			continue
		if is_relation or mdn == non_iso[eq_cube][0]:  # same mdn or is a relation
			return True
		else:
			print(f"***************************Found it {eq_cube}\n{non_iso[eq_cube]}")
	return False


def remove_isomorphic_old(cubes, is_relation, all_permutations):
	non_iso = dict()
	non_iso_unsorted = list()
	for x in cubes:
		mdn = find_cube_mdn(x)
		r = find_cube_radius(x)
		y = tuple(sorted(x))
		if not has_iso(y, is_relation, all_permutations, mdn, r, non_iso):
			non_iso_unsorted.append(tuple(x))
			non_iso[y] = [mdn, r]
	return tuple(non_iso_unsorted)


def remove_isomorphic(cubes, is_relation, all_permutations):
	"""
	is_relation (List[bool]): the operation is a relation
	"""
	non_iso = dict()
	non_iso_unsorted = list()
	for x in cubes:
		mdn = find_cube_mdn(x)
		r = find_cube_radius(x)
		y = tuple(sorted(x))
		if not has_iso(y, is_relation, all_permutations, mdn, r, non_iso):
			non_iso_unsorted.append(tuple(x))
			non_iso[y] = [mdn, r]
	return tuple(non_iso_unsorted)


def is_in_prev(all_cubes, x):
	for prefix in all_cubes:
		if x.startswith(prefix):
			return True
	return False
		
		
def gen_sequence(n, cube_length, arities, is_relation, all_permutations, prev_cubes_filepath, in_cubes_filepath, out_cubes_filepath):
	"""
		n (int): order of the algebra
		cube_length (int): length of the search sequence
		arities (List[int]): arity of each function
		is_relation (List[bool]): True if if the function symbol is a relation
		prev_cubes_filepath (str): file path of a file containing all cubes of shorter length to build on
		in_cubes_filepath (str): file path of the file containing the cubes generated by Mace4
		out_cubes_filepath (str): file for output cubes of length "cube_length"
		all_permutations (List[List[int, int]]): list of permutations
	"""	
	prev_str = read_cubes_file(prev_cubes_filepath)	
	in_cubes_str = read_cubes_file(in_cubes_filepath)
	
	# filter out those whose parents are already filtered out
	full_cubes_str = [x for x in in_cubes_str if is_in_prev(prev_str, x)]
	
	ordered_cells = gen_func_cells(cube_length, arities)
	
	print(ordered_cells)
	all_cubes = list()
	for cube_str in full_cubes_str:
		cell_values = [int(x) for x in cube_str.split(" ")]
		cube = list()
		for index, x in enumerate(ordered_cells):
			cube.append((x, cell_values[index]))
		all_cubes.append(cube)
	
	print(f"gen_sequence, starting number of cubes: {len(in_cubes_str)}, {len(ordered_cells)}", flush=True)
	seq = remove_isomorphic(all_cubes, is_relation, all_permutations)
	with (open(out_cubes_filepath, "w")) as fp:
		for cube in seq:
			for term in cube:
				fp.write(f"{term[1]} ")
			fp.write("\n")
	print(f"gen_sequence, final number of cubes: {len(seq)}, {len(ordered_cells)}")
		
		
def gen_sequence_1_2(n, cube_length, is_relation, all_permutations, prev_cubes_filepath, in_cubes_filepath, out_cubes_filepath):
	"""
		n (int): order of the algebra
		cube_length (int): length of the search sequence
		is_relation (bool): True if it is searching a relation
		prev_cubes_filepath (str): file path of a file containing all cubes of shorter length to build on
		in_cubes_filepath (str): file path of the file containing the cubes generated by Mace4
		out_cubes_filepath (str): file for output cubes of length "cube_length"
		all_permutations (List[List[int, int]]): list of permutations
	"""	
	prev_str = read_cubes_file(prev_cubes_filepath)	
	in_cubes_str = read_cubes_file(in_cubes_filepath)
	
	# filter out those whose parents are already filtered out
	full_cubes_str = [x for x in in_cubes_str if is_in_prev(prev_str, x)]
	
	ordered_cells = ordering_cells_1_2(cube_length)
	print(ordered_cells)
	all_cubes = list()
	for cube_str in full_cubes_str:
		cell_values = [int(x) for x in cube_str.split(" ")]
		cube = list()
		for index, x in enumerate(ordered_cells):
			cube.append((x, cell_values[index]))
		all_cubes.append(cube)
	
	print(f"gen_sequence_1_2, starting number of cubes: {len(in_cubes_str)}, {len(ordered_cells)}", flush=True)
	seq = remove_isomorphic(all_cubes, is_relation, all_permutations)
	with (open(out_cubes_filepath, "w")) as fp:
		for cube in seq:
			for term in cube:
				fp.write(f"{term[1]} ")
			fp.write("\n")
	print(f"gen_sequence_1_2, final number of cubes: {len(seq)}, {len(ordered_cells)}")

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


def gen_func_cells(cube_length, arities):
	r = 0
	cells = list()
	while len(cells) < cube_length:
		for index, a in enumerate(arities):
			new_cell = list()
			for x in range(0, a):
				new_cell.append(r)
			cells.append((index, tuple(new_cell)))
			if len(cells) > cube_length:
				break
		for index, a in enumerate(arities):
			if a == 1:
				continue
			new_cell = list()
			for x in range(0, r):         # a is assumed to be 2
				cells.append((index, tuple([x, r])))
			if len(cells) > cube_length:
				break
			for x in range(0, r):
				cells.append((index, tuple([r, x])))
			if len(cells) > cube_length:
				break
		r = r + 1
	return tuple(cells)
			
	

if __name__ == "__main__":	
	permutations2 = [[1, 0]]
	permutations3 = permutations2 + [[0,2,1],[1,2,0],[2,1,0],[2,0,1]]
	# permutations = [[0, 2, 1]]
	permutations4 =  shorten_permutations(list(permutations(range(0, 4))))   # permute 0, 1, 2, 3 only
	permutations5 = shorten_permutations(list(permutations(range(0, 5))))
	permutations6 = shorten_permutations(list(permutations(range(0, 6))))
	perm = {2: [], 4: permutations2, 6: permutations2, 12: permutations3, 20: permutations4, 30: permutations5}
	
	print(f"*********************{list(permutations(range(0, 3)))}")
	order = 6
	prev_cube_length = 0
	cube_length = 4

	algebra = "iploop"
	algebra = "disemi"
	
	arities = {"disemi": [2, 2]}
	is_relation = {"disemi": [False, False]}

	if algebra in ["hilbert", "semizero", "loops", "iploop"]:
		all_permutations = remove0(perm[cube_length])
	else:
		all_permutations = perm[cube_length]

	cube_file = f"{algebra}{order}/cubes{cube_length}.out"
	prev_file = f"{algebra}{order}/cubes_2_{order}_{prev_cube_length}.out"
	out_cube_file = f"{algebra}{order}/cubes_2_{order}_{cube_length}.out"

	print(f"order: {order}, cube_length: {cube_length}, previous file: {prev_file}")
	print(f"permutations: {all_permutations}")
	gen_sequence(order, cube_length, arities['disemi'], is_relation["disemi"], all_permutations, prev_file, cube_file, out_cube_file)
	



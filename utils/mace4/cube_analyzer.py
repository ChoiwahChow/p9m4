#!/usr/bin/python3

import sys
import copy
import json
	

def permute(s, a):
	if len(s) > a:
		return s[a]
	else:
		return a


def find_cube_radius(cube):
	r = 0
	for x in cube:
		for y in x[0]:
			if y > r:
				r = y
	return r


def find_cube_mdn(cube):
	m = 0
	for x in cube:
		if x[1] > m:
			m = x[1]
		for y in x[0]:
			if y > m:
				m = y
	return m


def equivalent_cube(cube, permutation):
	"""
		cube (List[List[int], int]): a cube
		permutations(List[int]): a permutation
	"""
	eq_cube = list()
	for x in cube:
		cell = tuple(permute(permutation, y) for y in x[0])
		term = tuple([cell, permute(permutation, x[1])])
		eq_cube.append(term)
	return tuple(sorted(eq_cube))				
					

def has_iso(x, permutations, mdn, r, non_iso):
	"""  check isomorphism against all given permutations
		x (List(List[int, int], int]]): a cube
		permuations (List[list[int]]): a list of permutations
		mdn (int):   mdn
		r (int):     radius
		non_iso (dict): dictionary of non-isomorphic cubes
	"""
	if x in non_iso and non_iso[x][0] == mdn:
		return True
	for p in permutations:
		eq_cube = equivalent_cube(x, p)
		if not eq_cube in non_iso:
			continue
		y = non_iso[eq_cube]
		if mdn == y[0]:
			return True
	return False


def ordering_cells_1(max_depth):
	"""
	max_depth (int): maximum depth of the search search, note count starts from 0,
					 so a max depth of 2 is 0 and 1.
	"""
	seq = list()
	d = 0
	while len(seq) < max_depth:
		seq.append((d))
		d += 1
	return tuple(seq);


def ordering_cells_2(max_depth):
	""" ordering: diagonal first, then minimum row, then minimum column,
	  then next minimum row, then next minimum column ...
	  e.g. (0, 0), (1, 1), (0, 1), (1, 0), (2, 2), (0, 2), (2, 0), (1, 2), (2, 1), (3, 3)...
	Args:
		max_depth (int): maximum depth of the search search, note count starts from 0,
						 so a max depth of 2 is 0 and 1.
	"""
	seq = list()
	d = 0
	while len(seq) < max_depth:
		seq.append((d, d))
		if len(seq) >= max_depth:
			return seq
		for x in range(0, d):
			seq.append((x, d))
			if len(seq) >= max_depth:
				return seq
			seq.append((d, x))
			if len(seq) >= max_depth:
				return seq
		d += 1
	return tuple(seq)


def extend_sequence(n, depth, mdn, ordered_cells, purged_cubes, branch, tree):
	"""
		n (int): order of the algebra
		depth (int): depth of the current search node
	"""
	if depth == len(ordered_cells):
		tree.append(branch)
		return 1, 0
	cell = ordered_cells[depth]
	mdn = max(mdn, max(cell))
	max_range = min(n-1, mdn+1)
	count = 0
	count_found = 0

	for x in range(max_range+1):
		new_branch = copy.deepcopy(branch)
		new_branch.append((cell, x))
		if tuple(sorted(new_branch)) in purged_cubes:
			count_found += 1
			continue
		count_1, count_found_1 = extend_sequence(n, depth+1, max(mdn, x), ordered_cells, purged_cubes, new_branch, tree)
		count += count_1
		count_found += count_found_1
	# print(f"*******************found {count_found} from {count}")
	return count, count_found


def remove_isomorphic(cubes, permutations):
	non_iso = dict()
	non_iso_unsorted = list()
	purged_cubes = list()
	for x in cubes:
		mdn = find_cube_mdn(x)
		r = find_cube_radius(x)
		y = tuple(sorted(x))
		if not has_iso(y, permutations, mdn, r, non_iso):
			non_iso_unsorted.append(tuple(x))
			non_iso[y] = [mdn, r]
		else:
			purged_cubes.append(y)
	return tuple(non_iso_unsorted), purged_cubes
		
		
def gen_sequence_2(n, depth, permutations, purged_cubes, purged_cubes_out_filepath, cubes_filepath):
	"""
		n (int): order of the algebra
		depth (int): maximum depth of the search sequence
		permutations (List[List[int, int]]): list of permutations
	"""
	mdn = -1
	ordered_cells = ordering_cells_2(depth)
	seq = list()
	extend_sequence(n, 0, mdn, ordered_cells, purged_cubes, [], seq)
	#for x in seq:
	#	print_short(x)
	print(f"gen_sequence_2, number of tables: {len(seq)}, {len(ordered_cells)}", flush=True)
	seq, additional_purged = remove_isomorphic(seq, permutations)
	if purged_cubes_out_filepath:
		write_purged_cubes(purged_cubes_out_filepath, list(purged_cubes)+additional_purged)
	with (open(cubes_filepath, "w")) as fp:
		for x in seq:
			fp.write(f"{x}\n")
	print(f"gen_sequence_2, number of tables: {len(seq)}, {len(ordered_cells)}")
					

def gen_sequence_1(n, depth, purged_cubes, purged_cubes_out_filepath, cubes_filepath):
	mdn = -1
	ordered_cells = ordering_cells_1(depth)
	# print(ordered_cells)
	seq = list()
	extend_sequence(n, 0, mdn, ordered_cells, purged_cubes, [], seq)
	if purged_cubes_out_filepath:
		write_purged_cubes(purged_cubes_out_filepath, list(purged_cubes)+additional_purged)
	with (open(cubes_filepath, "w")) as fp:
		for x in seq:
			fp.write(f"{x}\n")
	print(f"gen_sequence_1, number of tables: {len(seq)}, {len(ordered_cells)}")

def read_purged_cubes(file_path):
	with (open(file_path)) as fp:
		c = json.load(fp)
	p = [tuple(tuple([tuple(y[0]), y[1]]) for y in x) for x in c]
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
	

if __name__ == "__main__":	
	# permutations = [[1, 0]]
	# permutations = [[0, 2, 1]]
	# permutations =  [[0, 2, 1], [1, 0], [1, 2, 0], [2, 1, 0], [2, 0, 1]]
	permutations =  [[0, 2, 1], [1, 0], [1, 2, 0], [2, 1, 0], [2, 0, 1], 
					[0, 1, 3, 2], [0, 2, 3, 1], [0, 3, 1, 2], [0, 3, 2, 1],
					[1, 0, 3, 2], [1, 2, 3, 0], [1, 3, 0, 2], [1, 3, 2, 0],
					[2, 0, 3, 1], [2, 1, 3, 0], [2, 3, 0, 1], [2, 3, 1, 0],
					[3, 0, 2, 1], [3, 1, 2, 0], [3, 2, 0, 1], [3, 2, 1, 0]]
	order = 7
	cube_length = 14
	purged_file = "purged_cubes_2_7_12.json"
	out_purged_file = f"./purged_cubes_2_7_{cube_length}.json"
	out_cube_file = f"./cubes_2_7_{cube_length}.json"
	if purged_file:
		c = read_purged_cubes(purged_file)
		p = {x: 1 for x in c}
	else:
		p = dict()
	print(f"order: {order}, cube_length: {cube_length}, purged file: {purged_file}")
	print(f"permutations: {permutations}")
	gen_sequence_2(order, cube_length, permutations, p, out_purged_file, out_cube_file)
	

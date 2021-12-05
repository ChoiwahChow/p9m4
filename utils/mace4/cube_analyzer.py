#!/usr/bin/python3

import copy


def permute_0_1(x):
	if x in [0, 1]:
		return (x+1)%2
	else:
		return x


def apply_cycle_0_1(cell):
	""" cell is a list of lists - a square 2 x 2 matrix
	"""
	pcell = list()
	pcell = [[permute_0_1(cell[1][1]), permute_0_1(cell[1][0])]]
	pcell.extend([[permute_0_1(cell[0][1]), permute_0_1(cell[0][0])]])
	return pcell


def add_tail(d, n):
	if d == 0:
		return [[x] for x in range(0, n)]
	else:
		p = add_tail(d-1, n)
		q = copy.deepcopy(p)
		for x in range(0, len(p)):
			p[x].append(0)
			q[x].append(1)
		p.extend(q)
		return p
		
		
def list_all_cubes_for_square2(n):
	""" n is the side of the square  of the multiplication table
		The cube will be of length n^2
	"""
	x = add_tail(n*n-1, n)
	x = sorted(x)
	return [ [y[:n], y[n:]] for y in x]
	

def list_all_iso_0_1(x):
	""" x is the list of 2 x 2 matrix for multiplication table
		apply permutation by cycle (0, 1) to each of the cells
	"""
	return [ apply_cycle_0_1(y) for y in x ]
	

def remove_all_iso(x, y):
	""" x is a list of square matrices, and y is their corresponding iso
	"""
	non_iso = list()
	removed = list()
	for a, b in zip(x, y):
		if a in removed:
			continue
		removed.append(b)
		non_iso.append(a)
	return non_iso
	

def encode_cube(x):
	""" x is a square matrix
	"""
	flat_list = [str(item) for sublist in x for item in sublist]
	return "_".join(flat_list)
	
	
def find_max_constraints(n, start):
	""" n is the side of a multiplication table, which is an n x n matrx
		start is the starting max_constraint, assumed to be -1 or more
		Assume concentric selection
	"""
	row = [0] * n
	c = [copy.copy(row) for _ in range(0, n)]
	max_so_far = start
	for x in range(0, n):
		for y in range(0, x+1):
			c[y][x] = max(x, y, max_so_far)
			max_so_far += 1
			if x != y:
				c[x][y] = max(x, y, max_so_far)
				max_so_far += 1
	return c	


if __name__ == "__main__":
	x = list_all_cubes_for_square2(2);
	print(x)
	y = list_all_iso_0_1(x)
	print(y)
	non_iso = remove_all_iso(x, y)
	print(len(non_iso))
	print(non_iso)
	cube = [encode_cube(x) for x in non_iso]
	print(cube)
	
	for a, b in zip(x, y):
		print(a, b)

	print("Max constraints")
	c = find_max_constraints(2, 0)
	print(c)
	
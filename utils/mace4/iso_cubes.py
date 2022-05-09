#!/usr/bin/python3

import sys
import json
	

def permute(s, a):
	if len(s) > a:
		return s[a]
	else:
		return a


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


def remove_isomorphic_cubes(cubes, is_relation, all_permutations):
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


def remove_isomorphic_cubes_wrapper():
	params_json = json.load(sys.stdin)
	cubes = [[tuple([tuple([y[0][0], tuple(y[0][1])]), y[1]]) for y in x] for x in params_json['cubes']]
	is_relation = params_json['is_relation']
	all_permutations = params_json['all_permutations']
	outputs = remove_isomorphic_cubes(cubes, is_relation, all_permutations)
	# print(f"{outputs}")
	return outputs


__all__ = ["remove_isomorphic_cubes"]


if __name__ == "__main__":
	# non_iso = remove_isomorphic_cubes(cubes, is_relation, all_permutations)
	non_iso = remove_isomorphic_cubes_wrapper()
	print(json.dumps(non_iso))

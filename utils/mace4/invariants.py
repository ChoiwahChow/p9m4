#!/usr/bin/python3

from collections import defaultdict


def calc_binary_invariant_vec(radius, mt):    
    inv_vec = list()
    yy_square = [mt[y][y] for y in range(0, radius+1)]

    for el in range(0, radius+1):
        inv_vec.append(list())        
        
        # Invariant B2 The number of y in D such that x = (xy)x (number of inverses).
        inv_vec[el].append(len([y for y in range(0, radius+1) if mt[el][y] <= radius and el == mt[mt[el][y]][el]]))
        
        # Invariant B3: size of right ideal
        inv_vec[el].append(len(set([mt[el][x] for x in range(0, radius+1)])))
        
        # Invariant B4: size of left ideal
        inv_vec[el].append(len(set([mt[x][el] for x in range(0, radius+1)])))
    
        # Invariant B5: idempotent
        if mt[el][el] == el:
            inv_vec[el].append(1)
        else:
            inv_vec[el].append(0)

        # Invariant B6 The number of y in D such that x(yy) = (yy)x (number of commuting squares).
        inv_vec[el].append(len([y for y in range(0, radius+1) if yy_square[y] <= radius and mt[el][yy_square[y]] == mt[yy_square[y]][el]]))
        
        # Invariant B7: square root
        inv_vec[el].append(len([y for y in range(0, radius+1) if yy_square[y] == el]))
        
        # Invariant B8: The number of y ∈ D such that x(xy) = (xx)y (number of square associatizers).
        inv_vec[el].append(len([y for y in range(0, radius+1) if yy_square[el] <= radius and mt[el][y] <= radius and mt[el][mt[el][y]] == mt[yy_square[el]][y]]))
    
    return inv_vec
        
        
def calc_binary_relation_invariant_vec(radius, mt):
    inv_vec = list()

    for el in range(0, radius+1):
        inv_vec.append(list())
        inv_vec[el].append(len(set([y for y in range(0, radius+1) if mt[el][y]==1])))   # Invariant R1
        inv_vec[el].append(len(set([y for y in range(0, radius+1) if mt[y][el]==1])))   # Invariant R2
        inv_vec[el].append(mt[el][el])     # Invariant R3        
        inv_vec[el].append(len(set([y for y in range(0, radius+1) if mt[el][y]==1 and mt[y][el]==1])))   # Invariant R4
    
    return inv_vec


def calc_unary_invariant_vec(radius, mt):
    """
    Args:
        radius (int): radius of the cube
        mt (List[int]: multiplication table
    Returns:
        (List[List[int]]):
    """
    inv_vec = list()
    for el in range(0, radius+1):
        inv_vec.append(list())
        
    for el in range(0, radius+1):
        # Invariant U1
        if mt[el] == el:
            inv_vec[el].append(1)
        else:
            inv_vec[el].append(0)
        # Invariant U3
        inv_vec[el].append(len(set([y for y in range(0, radius+1) if mt[y] == el])))
        
    return inv_vec


def calc_combo_invariant_vec(mts, radius, arities, is_relation):
    # combo_vec is a list (vector) of lists (invariant vectors, one for each domain element)
    combo_vec = list()
    for el in range(0, radius+1):  # each domain element has an invariant vector (list)
        combo_vec.append(list())
    for pos, mt in enumerate(mts):
        if arities[pos] == 1:
            inv_vec = calc_unary_invariant_vec(radius, mt)
        elif is_relation[pos]:
            inv_vec = calc_binary_relation_invariant_vec(radius, mt)
        else:
            inv_vec = calc_binary_invariant_vec(radius, mt)
        for el in range(0, radius+1):
            combo_vec[el].extend(inv_vec[el])
    for el in range(0, radius+1):
        combo_vec[el] = ",".join([str(x) for x in combo_vec[el]])
    return ";".join(sorted(combo_vec))


def construct_mt(cube, radius, arities):
    """
    Args:
        cube:  [[((0, (0, 0)), 0), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], [((0, (0, 0)), 1), ((1, (0, 0)), 0), ((2, (0, 0)), 0)]]
    """
    mts = list()
    for x in range(0, len(arities)):
        mt = list()
        for y in range(0, radius+1):
            if arities[x] == 1:
                mt.append(-1)
            else:
                mt.append(list())
                for z in range(0, radius+1):
                    mt[y].append(-1)                
        mts.append(mt)
    # print(f"*************mts {len(mts)}")
    for x in cube:
        func = x[0][0]
        cell = x[0][1]
        val = x[1]
        if len(cell) == 1:
            mts[func][cell[0]] = val
        else:
            # print(f"***********looking for {cell}")
            mts[func][cell[0]][cell[1]] = val
        
    return mts


def calc_invariant_vec(cubes, radius, arities, is_relation):
    """ Entry point for the module to partition a set of cubes by their invariants
        A cube is represented by a list of (cell term, value), and in addition, there is a number of cells filled for each cube.
        e.g. cubes:  [[[((0, (0, 0)), 0), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], 8], [[((0, (0, 0)), 1), ((1, (0, 0)), 0), ((2, (0, 0)), 0)], 7]]
    Args:
        cubes (List[cube]): list of cubes
        radius (int): radius of the cubes
        arities (List[int]): arity of each operation
        is_relation (List[bool]): whether the operation is a relation
    """
    blocks = defaultdict(list)
    for cube in cubes:
        # calc invariants
        mts = construct_mt(cube[0], radius, arities)
        key = calc_combo_invariant_vec(mts, radius, arities, is_relation)
        # hashing
        blocks[key].append(cube)
    
    return blocks

__all__ = ["calc_invariant_vec"]

    
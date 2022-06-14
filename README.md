# p9m4
Prover9/Mace4 with LADR library. But mainly for Mace4.

## Mace4

### Cell Selection

The function/relation is implemented as a linearized array.  For example, an n x n 2-d function table is implemented
as a 1-d n^2 array. 

Each cell (a, b) in the 2-d function table has indices a and b, and a max-index of max(a, b).  Constants are implemented
as 0-ary functions, and have max index of zero.  All the cells of the
functions and relations are ordered in ascending order by their max indices in the `Ordered_Cells`.

Note that skolemization constants can be at the beginning of the list or at the end of the list (default).

#### Cell Selection Order

The list of cells to be considered for the next assignment are determined by one of the following strategies:

1. SELECT_LINEAR: All open cells in `Ordered_Cells` are available for selection.
2. SELECT_CONCENTRIC: All open cells having the same max index as the first open cell in `Ordered_Cells` are available for selection.
3. SELECT_CONCENTRIC_BAND (default): All open cells having the some specified max index in `Ordered_Cells` are available for selection
4. SELECT_BY_ORDER: The first open cell in `Ordered_Cells` can be used.  This is added for doing cube-and-conquer.

Among these available cells, the cell with the best `selection measure` (see section on the Cell Selection Measure below) will be selected for 
assigning values.

Note that there are asymmetries for the `SELECT_CONCENTRIC` strategy.  For example, there are 2 binary functions in the search, and the 
cells in the second binary function are preferred by the `selection measure` below. All the cells in the second binary function of a `max index` will be selected
first, followed by the cells of the same `max index` of the first binary function.  However, if the cells in the first binary function
of a `max index` is preferred over those of the second binary function, then all cells of the first binary function will
be selected before the first cell in the second binary function will be selected.


#### Cell Selection Measure

The next cell to select for assignment of value is the one with the highest score according to some selection measure calculated
by the function `Selection::selection_measure`.  There are options on how the measures are dynamically calculated:

1. MOST_OCCURRENCES: The cell with most number of occurrences in the active clauses.
2. MOST_PROPAGATIONS: The cell with the most total number of propagations for all the assignable values. 
3. MOST_CONTRADICTIONS: The cell with the maximum number of contradictions for all assignable values. 
4. MOST_CROSSED (default): The cell with most assignable values crossed off.

For (2) and (3), it actually does the assignments and counts the number of propagations/contradictions, so they have higher overheads.



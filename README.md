# p9m4
Prover9/Mace4 with LADR library. But mainly for Mace4.

## Mace4

## Integration of Nauty (sparse mode) into Mace4 
Mace4 can internally do isomorphic model filtering using Nauty (sparse mode) to output non-isomorphic models.

E.g.:
```text
mace4 -n5 -N5 -m-1 -A-1 -W-1 -f <input-file>
```

The option -W tells Mace4 to do isomorphic model filter. `0` means the feature is off (default is 0), -1 means no 
restriction on the number of non-isomorphic model cached.  `-W1000`, for example, means Mace4 keeps only the first 1000 non-isomorphic
models in its memory. Models that are not isomorphic to these 1000 models are printed out as non-isomorphic models
although some of them can be isomorphic to one another.  Additional rounds of external isofiltering will be needed to filter out the rest
of the isomorphic models.  This is to limit the size of the Mace4 so that more copies of Mace4's can run in parallel.

Another useful option is `-w1` (defaults 0) tells Mace4 to output the canonical graphs of the non-isomorphic models in addtion
to the models in Mace4 format. This is useful if we need to check for isomorphism of models from different copies of Mace4 run
in parallel.  We can compare the canonical graphs without running nauty again.

The option `-a` sets the file path of the models output file. The default is `models.out`.

The option `-x` sets the file path of an external script to process the models outputted so far.  The option`-X` sets the number of models outputed before call the extern script to process those models.  Once the external script is called, `mace` resets the output file to an empty file, and resets the count of models outputted so far to zero.

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
2. SELECT_CONCENTRIC: All open cells having the same `max index` as the first open cell in `Ordered_Cells` are available for selection.
3. SELECT_CONCENTRIC_BAND (default): All open cells having `max index` the same as, or less than, the specified `max constrained` number are available for selection.
4. SELECT_BY_ORDER: Only the first open cell in `Ordered_Cells` can be used.  This is added for doing cube-and-conquer.

In the `SELECT_CONCENTRIC_BAND` strategy, `max constrained` is the maximal designated number (`mdn`). That is, any cell that does not increase the `mdn` is allowed in this strategy.

From this list of available cells, the cell with the best `selection measure` (see the section on the Cell Selection Measure below) will be selected for 
assigning values.


#### Cell Selection Measure

The next cell to select for assigning values is the one with the highest score according to some selection measure calculated
by the function `Selection::selection_measure`.  These are the options on how the measures are dynamically calculated:

1. MOST_OCCURRENCES: The cell with most number of occurrences in the active clauses.
2. MOST_PROPAGATIONS: The cell with the most total number of propagations for all the assignable values. 
3. MOST_CONTRADICTIONS: The cell with the maximum number of contradictions for all assignable values. 
4. MOST_CROSSED (default): The cell with most assignable values crossed off.  That is, the slimmest subtree wins.

For (2) and (3), it actually does the assignments and counts the number of propagations/contradictions, so they have higher overheads.


The Mace4 manual states that there are no heuristics to decide which selection order to use, and suggest to try different ones when the current strategy is slow.





#ifndef MACE4_ORDERCELL_H
#define MACE4_ORDERCELL_H

#include "../ladr/order.h"
#include "../ladr/term.h"

#include "syms.h"


typedef struct cell*        Cell;  // strangely, this is not in the original Mace4 code

/* A cell is member of a function or relation table.  For example,
   if we have a group operation and the domain is size 6, then
   the term f(2,3) represents one cell of the group multiplication
   table.  We will dynamically allocate a (1-dimensional) array
   of cells (to hold all tables) at the beginning of the run after
   we know how many we'll need.  The array will be indexed by IDs
   that are constructed from the terms representing the cells.
   (Each symbol has a "base" value, and the cell ID is an offset
   from the base.)  We have, in effect, multidimensional arrays
   within our main array of cells, we calculate the IDs from
   the symbol base and the term arguments.  Anyway, given a term
   like f(2,3), in which all of the arguments are domain members,
   we can quickly get to the corresponding cell.

   Thread-safe?  No, need to get rid of static bool Skolems_last
*/

class cell {   // orig from msearch.h, but most functions are from ordercells.c
private:
  int   id;
  Term  eterm;          /* the term representation, e.g., f(2,3) */
  Term  value;          /* current value of cell (domain element or NULL) */
  Term  occurrences;    /* current occurrences of the term */
  Term* possible;       /* current set of possible values */
  int   max_index;      /* maximum index for this cell */
  Symbol_data symbol;   /* data on the function or relation symbol */

public:
  cell() : symbol(nullptr), max_index(0), possible(nullptr), occurrences(nullptr), value(nullptr), eterm(nullptr), id(0) {}
  inline size_t get_base() const {return symbol->get_base();}
  inline int get_sn() const {return symbol->get_sn();}
  inline bool has_value() const {return value != nullptr;}
  inline int get_value() const {return value->private_symbol;}
  inline int get_id() const {return id;}
  inline int get_index(int pos) const {if (ARITY(eterm) > pos) return VARNUM(ARG(eterm, pos)); else return -1;}
  inline std::string& get_symbol() const {return Symbol_dataContainer::get_op_symbol(get_sn()); }

public:
  friend class Ground;
  friend class propagate;
  friend class Selection;
  friend class Search;
  friend class CellContainer;
};

class CellContainer {
private:
  static bool Skolems_last;  // TODO: [Choiwah] we may need to get rid of this member variable to make it thread-safe, although it is set only once

private:
  static int  sum_indexes(Term t);
  static bool equal_index(Term t);
  static OrderType compare_cells(Cell a, struct cell* b);

public:
  static int id_to_domain_size(int id, Cell Cells, int Domain_size);
  static int order_cells(bool verbose, Cell Cells, int Number_of_cells, bool Skolems_last, Cell Ordered_cells[]);

};

#endif



#include "../ladr/parse.h"
#include "cell.h"


bool CellContainer::Skolems_last = false;


int
CellContainer::id_to_domain_size(int id, Cell Cells, int Domain_size)
{
  // Originally in syms.c
  /* Assume the id is in range. */
  Symbol_data s = Cells[id].symbol;
  return (s->get_type() == type_RELATION ? 2 : Domain_size);
}

int
CellContainer::sum_indexes(Term t)
{
  // Assume t is an eterm, that is, non-variable with variable arguments.
  if (ARITY(t) == 0)
    return -1;
  else {
    int sum = 0;
    for (int i = 0; i < ARITY(t); i++)
      sum += VARNUM(ARG(t,i));
    return sum;
  }
}

OrderType
CellContainer::compare_cells(Cell a, Cell b)
{
  // TODO: [Choiwah] we need to change the compare_cells prototype to include an extra
  // param to hold Skolems_last.  This requires a change in merge_sort to include an optional param void*
  if (a->symbol->attribute == EQUALITY_SYMBOL &&
      b->symbol->attribute != EQUALITY_SYMBOL)       return OrderType::GREATER_THAN;

  else if (a->symbol->attribute != EQUALITY_SYMBOL &&
           b->symbol->attribute == EQUALITY_SYMBOL)  return OrderType::LESS_THAN;

  else if (Skolems_last &&
           a->symbol->attribute == SKOLEM_SYMBOL &&
           b->symbol->attribute != SKOLEM_SYMBOL)    return OrderType::GREATER_THAN;

  else if (Skolems_last &&
           a->symbol->attribute != SKOLEM_SYMBOL &&
           b->symbol->attribute == SKOLEM_SYMBOL)    return OrderType::LESS_THAN;

  else if (a->max_index < b->max_index)              return OrderType::LESS_THAN;

  else if (a->max_index > b->max_index)              return OrderType::GREATER_THAN;

  else if (a->symbol->mace_sn < b->symbol->mace_sn)  return OrderType::LESS_THAN;

  else if (a->symbol->mace_sn > b->symbol->mace_sn)  return OrderType::GREATER_THAN;

  else if (sum_indexes(a->eterm) <
           sum_indexes(b->eterm))                    return OrderType::LESS_THAN;

  else if (sum_indexes(a->eterm) >
           sum_indexes(b->eterm))                    return OrderType::GREATER_THAN;

  else
    return OrderType::SAME_AS;  /* For now, let f(0,1) be the same as f(1,0), etc.  */

}

int
CellContainer::order_cells(bool verbose, Cell Cells, int Number_of_cells, bool Skolems_last, Cell Ordered_cells[])
{
  int First_skolem_cell = Number_of_cells;

  for (int i = 0; i < Number_of_cells; i++)
    Ordered_cells[i] = Cells + i;

  CellContainer::Skolems_last = Skolems_last;

  myOrder::merge_sort((void**)Ordered_cells, Number_of_cells,
                      (OrderType (*) (void*,void*))compare_cells);

  if (Skolems_last) {
    int i;
    for (i = 0; i < Number_of_cells; i++)
      if (Ordered_cells[i]->symbol->attribute == SKOLEM_SYMBOL)
        break;
    First_skolem_cell = i;  /* if none, set to Number_of_cells */
  }
  else
    First_skolem_cell = Number_of_cells;

  if (verbose) {
    /* print the Ordered_cells */
    SymbolContainer sym_con;
    ParseContainer  p_con;
    std::cout << "\n% Cell selection order:\n\n";
    for (int i = 0; i < Number_of_cells; i++) {
      Term t = Ordered_cells[i]->eterm;
      if (!sym_con.is_eq_symbol(SYMNUM(t))) {
        p_con.fwrite_term_nl(std::cout, t);
      }
    }
    std::cout << std::flush;
  }

  return First_skolem_cell;
}


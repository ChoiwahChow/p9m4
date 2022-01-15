
#include "../ladr/ladrvglobais.h"
#include "propagate.h"
#include "select.h"


Selection::Selection(int ds, const Term* dn, const Cell c, EstackContainer* e, struct mace_stats* m, const Mace_options o) :
  Domain_size(ds), Domain(dn), Cells(c), EScon(e), Mstats(m), Opt(o)
{

}

int
Selection::num_contradictions(int id, int max_so_far, propagate* prop)
{
  /*
   *   num_contradictions() - for a given ID, try all assignments and see
   *   how many give contradiction by propagation.
   *
   *   Leave the Propagations statistic as it was.
   *
   *   We're not interested in a number less than max_so_far, and
   *   if we determine that we'll never get that many, we return -1.
   */
  int n = 0;
  int save_prop_count = Mstats->propagations;
  int ds = CellContainer::id_to_domain_size(id, Cells, Domain_size);
  for (int i = 0; i < ds; i++) {
    Estack stk = prop->assign_and_propagate(id, Domain[i]);
    if (stk == nullptr)
      n++;
    else
      EScon->restore_from_stack(stk);

    int to_try = (ds - i) - 1;
    if (n + to_try <= max_so_far) {
      Mstats->propagations = save_prop_count;
      return -1;  /* can't beat max_so_far */
    }
  }
  Mstats->propagations = save_prop_count;
  return n;
}

int
Selection::num_propagations(int id, propagate* prop)
{
  /*
   *   num_propagations() - for a given ID, try all assignments
   *   and return the total number of propagations.
   *
   *   Leave the Propagations statistic as it was.
   */
  int n = 0;
  int save_prop_count = Mstats->propagations;
  int ds = CellContainer::id_to_domain_size(id, Cells, Domain_size);
  Mstats->propagations = 0;
  for (int i = 0; i < ds; i++) {
    Estack stk = prop->assign_and_propagate(id, Domain[i]);
    EScon->restore_from_stack(stk);
    n += Mstats->propagations;
    Mstats->propagations = 0;
  }
  Mstats->propagations = save_prop_count;
  return n;
}

int
Selection::num_crossed(int id)
{
  /*
   *   num_crossed() - number of values crossed off for a given ID.
   */
  Term *p = Cells[id].possible;
  int n = 0;
  int ds = CellContainer::id_to_domain_size(id, Cells, Domain_size);
  for (int i = 0; i < ds; i++)
    if (p[i] == NULL)
      n++;
  return n;
}

int
Selection::num_occurrences(int id)
{
  /*
   *   num_occurrences() - number of (active) occurrences of an eterm
   */
  int n = 0;
  for (Term t = Cells[id].occurrences; t != nullptr; t = static_cast<Term>(t->u.vp))
    n++;
  return n;
}

void
Selection::selection_measure(int id, int *max, int *max_id, propagate* prop)
{
  int n = 0;
  switch (LADR_GLOBAL_OPTIONS.parm(Opt->selection_measure)) {
  case MOST_OCCURRENCES:    n = num_occurrences(id);                break;
  case MOST_PROPAGATIONS:   n = num_propagations(id, prop);         break;
  case MOST_CONTRADICTIONS: n = num_contradictions(id, *max, prop); break;
  case MOST_CROSSED:        n = num_crossed(id);                    break;
  default: fatal::fatal_error("selection_measure: bad selection measure");
  }
  if (n > *max) {
    *max = n;
    *max_id = id;
  }
}

int
Selection::select_linear(int min_id, int max_id, propagate* prop)
{
  if (LADR_GLOBAL_OPTIONS.parm(Opt->selection_measure) == NO_MEASURE) {
    /* Return the first open cell. */
    int i = min_id;
    while (i <= max_id && Cells[i].value != NULL)
      i++;
    return (i > max_id ? -1 : i);
  }
  else {
    int id_of_max = -1;
    int max = -1;
    for (int i = min_id; i <= max_id; i++) {
      if (Cells[i].value == nullptr) {
        selection_measure(i, &max, &id_of_max, prop);
      }
    }
    return id_of_max;
  }
}

int
Selection::select_concentric(int min_id, int max_id, Cell Ordered_cells[], propagate* prop)
{
  /*
   *   This assumes that Ordered_cells is ordered FIRST by max_index.
   *
   *   If the first open cell has max_index n, return the best open
   *   cell with max index n.
   */

  // Find the first open cell.
  int i = min_id;
  while (i <= max_id && Ordered_cells[i]->value != nullptr)
    i++;
  if (i > max_id)
    return -1;
  else {
    /* Find the best cell with the same max_index as the first open cell. */
    int n = Ordered_cells[i]->max_index;
    int max_val = -1;
    int id_of_max = -1;

    while (i <= max_id && Ordered_cells[i]->max_index <= n) {
      if (Ordered_cells[i]->value == nullptr)
        selection_measure(Ordered_cells[i]->id, &max_val, &id_of_max, prop);
      i++;
    }
    return id_of_max;
  }
}

// added for cube-and-conquer, cell selection is strictly as ordered by indices of the cells
int
Selection::select_by_order(int min_id, int max_id, Cell Ordered_cells[])
{
  int i = min_id;
  while (i <= max_id && Ordered_cells[i]->value != nullptr)
	i++;
  if (i <= max_id) {
	// std::cout << "********************" << Ordered_cells[i]->id << std::endl;
	return Ordered_cells[i]->id;
  }
  else
	return -1;
}

int
Selection::select_concentric_band(int min_id, int max_id, int max_constrained, Cell Ordered_cells[], propagate* prop)
{
  int max = -1;
  int id_of_max = -1;
  int i = min_id;

  while (i <= max_id &&  Ordered_cells[i]->max_index <= max_constrained) {
    if (Ordered_cells[i]->value == nullptr)
      selection_measure(Ordered_cells[i]->id, &max, &id_of_max, prop);
    i++;
  }
  if (id_of_max >= 0) {
    return id_of_max;
  }
  else
    /* There is nothing in the band, so revert to select_concentric.
       This is a bit redundant, because it will scan (again) the full cells.
    */
    return select_concentric(min_id, max_id, Ordered_cells, prop);
}

int
Selection::select_cell(int max_constrained, int First_skolem_cell, int Number_of_cells, Cell Ordered_cells[], propagate* prop)
{
  int id = -1;
  switch (LADR_GLOBAL_OPTIONS.parm(Opt->selection_order)) {
  case SELECT_LINEAR: id = select_linear(0, First_skolem_cell-1, prop); break;
  case SELECT_CONCENTRIC: id = select_concentric(0, First_skolem_cell-1, Ordered_cells, prop); break;
  case SELECT_CONCENTRIC_BAND: id = select_concentric_band(0, First_skolem_cell-1, max_constrained, Ordered_cells, prop); break;
  case SELECT_BY_ORDER: id = select_by_order(0, First_skolem_cell-1, Ordered_cells); break;   // added for cube-and-conquer
  default: fatal::fatal_error("bad selection order");
  }

  if (id >= 0)
    return id;

  switch (LADR_GLOBAL_OPTIONS.parm(Opt->selection_order)) {
  case SELECT_LINEAR: id = select_linear(First_skolem_cell, Number_of_cells-1, prop); break;
  case SELECT_CONCENTRIC: id = select_concentric(First_skolem_cell, Number_of_cells-1, Ordered_cells, prop); break;
  case SELECT_CONCENTRIC_BAND: id = select_concentric_band(First_skolem_cell, Number_of_cells-1, max_constrained, Ordered_cells, prop); break;
  case SELECT_BY_ORDER: id = select_by_order(0, Number_of_cells-1, Ordered_cells); break;   // added for cube-and-conquer
  default: fatal::fatal_error("bad selection order");
  }

  return id;
}


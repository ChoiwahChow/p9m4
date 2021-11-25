

#ifndef MACE4_SELECT_H
#define MACE4_SELECT_H

#include "../ladr/options.h"
#include "cell.h"
#include "estack.h"
#include "mstats.h"
#include "options.h"
#include "propagate.h"

class Selection {
public:
  static constexpr int SELECT_LINEAR          = 0;  /* selection orders */
  static constexpr int SELECT_CONCENTRIC      = 1;
  static constexpr int SELECT_CONCENTRIC_BAND = 2;

  static constexpr int NO_MEASURE          = 0;    /* selection measures */
  static constexpr int MOST_OCCURRENCES    = 1;
  static constexpr int MOST_PROPAGATIONS   = 2;
  static constexpr int MOST_CONTRADICTIONS = 3;
  static constexpr int MOST_CROSSED        = 4;

private:
  /*
   * The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object does not own any of them.  They are "pointers" passed in.
   */
  const Term*        Domain;
  int                Domain_size;
  const Cell         Cells;
  const Mace_options Opt;
  struct mace_stats* Mstats;
  EstackContainer*   EScon;

public:
  Selection() = delete;
  Selection(const Selection&) = delete;
  Selection& operator=(const Selection&) = delete;

  Selection(int ds, const Term* dn, const Cell c, EstackContainer* e, struct mace_stats* m, const Mace_options o);

private:
  int  num_contradictions(int id, int max_so_far, propagate* prop);
  int  num_propagations(int id, propagate* prop);
  int  num_crossed(int id);
  int  num_occurrences(int id);
  void selection_measure(int id, int *max, int *max_id, propagate* prop);

public:
  int select_linear(int min_id, int max_id, propagate* prop);
  int select_concentric(int min_id, int max_id, Cell Ordered_cells[], propagate* prop);
  int select_concentric_band(int min_id, int max_id, int max_constrained, Cell Ordered_cells[], propagate* prop);
  int select_cell(int max_constrained, int First_skolem_cell, int Number_of_cells, Cell Ordered_cells[], propagate* prop);
};

#endif


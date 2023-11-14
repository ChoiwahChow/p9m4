

#ifndef MACE4_MSTATS_H
#define MACE4_MSTATS_H

#include "../ladr/clock.h"

struct mace_stats {
  /* stats for the current domain size */
  unsigned
    current_models,
    selections,
    assignments,
    propagations,
    cross_offs,
    rewrite_terms,
    rewrite_bools,
    indexes,
    ground_clauses_seen,
    ground_clauses_kept,
    rules_from_neg,

    neg_elim_attempts,
    neg_elim_agone,
    neg_elim_egone,

    neg_assign_attempts,
    neg_assign_agone,
    neg_assign_egone,

    neg_near_assign_attempts,
    neg_near_assign_agone,
    neg_near_assign_egone,

    neg_near_elim_attempts,
    neg_near_elim_agone,
    neg_near_elim_egone,

    // cubes
    num_cubes,
    num_cubes_cut;

  mace_stats();

  void reset_current_stats(void);
  void p_stats(int Domain_size, myClock& Mace4_clock);
};


#endif


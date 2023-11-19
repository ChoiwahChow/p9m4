
#include <iomanip>
#include "../ladr/banner.h"
#include "mstats.h"

mace_stats::mace_stats()
{
  reset_current_stats();
}

void
mace_stats::reset_current_stats(void)
{
  current_models = 0;
  selections = 0;
  assignments = 0;
  propagations = 0;
  cross_offs = 0;
  rewrite_terms = 0;
  rewrite_bools = 0;
  indexes = 0;
  ground_clauses_seen = 0;
  ground_clauses_kept = 0;
  rules_from_neg = 0;

  neg_elim_attempts = 0;
  neg_elim_agone = 0;
  neg_elim_egone = 0;

  neg_assign_attempts = 0;
  neg_assign_agone = 0;
  neg_assign_egone = 0;

  neg_near_assign_attempts = 0;
  neg_near_assign_agone = 0;
  neg_near_assign_egone = 0;

  neg_near_elim_attempts = 0;
  neg_near_elim_agone = 0;
  neg_near_elim_egone = 0;

  // cubes
  num_cubes = 0;
  num_cubes_cut = 0;
}


void
mace_stats::p_stats(int Domain_size, myClock& Mace4_clock)
{
  banner::print_separator(std::cout, "STATISTICS", true);

  std::cout << "\nFor domain size " << Domain_size << ".\n\n";

  std::cout << "Current CPU time: " << std::setprecision(2) << Mace4_clock.clock_seconds() << " seconds ";
  std::cout << "(total CPU time: " << std::setprecision(2) << myClock::user_seconds() << " seconds).\n";
  std::cout << "Ground clauses: seen=" << ground_clauses_seen << ", kept=" << ground_clauses_kept << ".\n";
  std::cout << "Selections=" << selections << ", assignments=" << assignments << ", propagations="
            << propagations << ", current_models=" << current_models << ".\n";
  std::cout << "Rewrite_terms=" << rewrite_terms << ", rewrite_bools=" << rewrite_bools << ", indexes=" << indexes << ".\n";
  std::cout << "Rules_from_neg_clauses=" << rules_from_neg << ", cross_offs=" << cross_offs << ".\n";
  std::cout << "Number of cubes processed: " << num_cubes << " Number of cubes cut: "  << num_cubes_cut << std::endl;
#if 0
  std::cout << "Negative propagation:\n";
  std::cout << "                 attempts      agone      egone\n";
  std::cout << "Neg_elim        " << std::setw(10) << neg_elim_attempts << " " << std::setw(10) << neg_elim_agone
            << " " << std::setw(10) << neg_elim_egone << "\n";
  std::cout << "Neg_assign      " << std::setw(10) << neg_assign_attempts << " " << std::setw(10) << neg_assign_agone
            << " " << std::setw(10) << neg_assign_egone << "\n";
  std::cout << "Neg_near_elim   " << std::setw(10) << neg_near_elim_attempts << " " << std::setw(10)
            << neg_near_elim_agone << " " << std::setw(10) << neg_near_elim_egone << "\n";
  std::cout << "Neg_near_assign " << std::setw(10) << neg_near_assign_attempts << " " << std::setw(10) << neg_near_assign_agone
            << " " << std::setw(10) << neg_near_assign_egone << "\n";
#endif
  banner::print_separator(std::cout, "end of statistics", true);
}

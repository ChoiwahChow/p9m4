
#include "options.h"
#include "../ladr/ladrvglobais.h"

mace_options::mace_options()
{
  // Initialize with default MACE4 options
  domain_size       = LADR_GLOBAL_OPTIONS.init_parm("domain_size",       0,     0, INT_MAX);
  start_size        = LADR_GLOBAL_OPTIONS.init_parm("start_size",        2,     2, INT_MAX);
  end_size          = LADR_GLOBAL_OPTIONS.init_parm("end_size",         -1,    -1, INT_MAX);
  iterate_up_to     = LADR_GLOBAL_OPTIONS.init_parm("iterate_up_to",    -1,    -1, INT_MAX);
  max_models        = LADR_GLOBAL_OPTIONS.init_parm("max_models",        1,    -1, INT_MAX);
  max_seconds       = LADR_GLOBAL_OPTIONS.init_parm("max_seconds",      -1,    -1, INT_MAX);
  max_seconds_per   = LADR_GLOBAL_OPTIONS.init_parm("max_seconds_per",  -1,    -1, INT_MAX);
  selection_order   = LADR_GLOBAL_OPTIONS.init_parm("selection_order",   2,     0, 3);
  selection_measure = LADR_GLOBAL_OPTIONS.init_parm("selection_measure", 4,     0, 4);
  increment         = LADR_GLOBAL_OPTIONS.init_parm("increment",         1,     1, INT_MAX);
  max_megs          = LADR_GLOBAL_OPTIONS.init_parm("max_megs",          500,  -1, INT_MAX);
  report_stderr     = LADR_GLOBAL_OPTIONS.init_parm("report_stderr",     -1,   -1, INT_MAX);
  print_cubes       = LADR_GLOBAL_OPTIONS.init_parm("print_cubes",       -2,   -2, INT_MAX);
  cubes_options     = LADR_GLOBAL_OPTIONS.init_parm("cubes_options",     0,     0, INT_MAX);

  print_models_interp    = LADR_GLOBAL_OPTIONS.init_flag("print_models_interp",    false);
  print_models           = LADR_GLOBAL_OPTIONS.init_flag("print_models",           true);
  print_models_tabular   = LADR_GLOBAL_OPTIONS.init_flag("print_models_tabular",   false);
  lnh                    = LADR_GLOBAL_OPTIONS.init_flag("lnh",                    true);
  trace                  = LADR_GLOBAL_OPTIONS.init_flag("trace",                  false);
  negprop                = LADR_GLOBAL_OPTIONS.init_flag("negprop",                true);
  neg_assign             = LADR_GLOBAL_OPTIONS.init_flag("neg_assign",             true);
  neg_assign_near        = LADR_GLOBAL_OPTIONS.init_flag("neg_assign_near",        true);
  neg_elim               = LADR_GLOBAL_OPTIONS.init_flag("neg_elim",               true);
  neg_elim_near          = LADR_GLOBAL_OPTIONS.init_flag("neg_elim_near",          true);
  verbose                = LADR_GLOBAL_OPTIONS.init_flag("verbose",                false);
  integer_ring           = LADR_GLOBAL_OPTIONS.init_flag("integer_ring",           false);
  order_domain           = LADR_GLOBAL_OPTIONS.init_flag("order_domain",           false);
  arithmetic             = LADR_GLOBAL_OPTIONS.init_flag("arithmetic",             false);
  iterate_primes         = LADR_GLOBAL_OPTIONS.init_flag("iterate_primes",         false);
  iterate_nonprimes      = LADR_GLOBAL_OPTIONS.init_flag("iterate_nonprimes",      false);
  skolems_last           = LADR_GLOBAL_OPTIONS.init_flag("skolems_last",           false);
  return_models          = LADR_GLOBAL_OPTIONS.init_flag("return_models",          false);

  iterate = LADR_GLOBAL_OPTIONS.init_stringparm("iterate",
                                                5,
                                                std::string("all"), std::string("evens"), std::string("odds"), std::string("primes"), std::string("nonprimes"));

  /* dependencies */
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models_interp, true, print_models, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models_interp, true, print_models_tabular, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models_tabular, true, print_models, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models_tabular, true, print_models_interp, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models, true, print_models_tabular, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(print_models, true, print_models_interp, false);

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(iterate_primes, true, iterate_nonprimes, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(iterate_nonprimes, true, iterate_primes, false);

  LADR_GLOBAL_OPTIONS.parm_parm_dependency(domain_size, start_size, 1, true);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(domain_size, end_size, 1, true);

  LADR_GLOBAL_OPTIONS.parm_parm_dependency(iterate_up_to, end_size, 1, true);

  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(iterate_primes, true, iterate, "primes");
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(iterate_nonprimes, true, iterate, "nonprimes");

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(integer_ring, true, lnh, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(order_domain, true, lnh, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(arithmetic, true, lnh, false);

  LADR_GLOBAL_OPTIONS.flag_parm_dependency(arithmetic, true, selection_order, 0);
}



#ifndef MACE4_OPTIONS_H
#define MACE4_OPTIONS_H

typedef struct mace_options* Mace_options;

struct mace_options {
  /* This structure holds the option IDs, not the option values! */

  /* flags */
  int print_models_interp;
  int print_models;
  int print_models_tabular;
  int print_cubes;
  int lnh;
  int trace;
  int negprop;
  int neg_assign;
  int neg_assign_near;
  int neg_elim;
  int neg_elim_near;
  int verbose;
  int integer_ring;
  int order_domain;
  int arithmetic;
  int iterate_primes;
  int iterate_nonprimes;
  int skolems_last;
  int return_models;  /* special case */

  /* parms */
  int domain_size;
  int start_size;
  int end_size;
  int iterate_up_to;
  int increment;
  int max_models;
  int selection_order;
  int selection_measure;
  int max_seconds;
  int max_seconds_per;
  int max_megs;
  int report_stderr;

  /* stringparms */
  int iterate;

  mace_options();
};

#endif


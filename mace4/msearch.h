
#ifndef MACE4_MSEARCH_H
#define MACE4_MSEARCH_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../ladr/clock.h"
#include "../ladr/symbols.h"
#include "../ladr/top_input.h"
#include "mace4globalvalues.h"
#include "mace4vglobais.h"
#include "propagate.h"
#include "partition.h"
#include "cube.h"

#include "syms.h"

enum MACE_EXIT_CODE {
  MAX_MODELS_EXIT   = 0,
  /* FATAL_EXIT     = 1,   declared elsewhere, and not needed here*/
  EXHAUSTED_EXIT    = 2,
  ALL_MODELS_EXIT   = 3,
  MAX_SEC_YES_EXIT  = 4,
  MAX_SEC_NO_EXIT   = 5,
  MAX_MEGS_YES_EXIT = 6,
  MAX_MEGS_NO_EXIT  = 7,

  MACE_SIGINT_EXIT  = 101,
  MACE_SIGSEGV_EXIT = 102
};

/* Mace results */

typedef struct mace_results* Mace_results;

struct mace_results {
  bool   success;
  Plist  models;
  double user_seconds;
  int    return_code;
};

class Search {
private:
  inline int X0(int b) {return b;}
  inline int X1(int b, int i) {return b + i;}
  inline int X2(int b, int i, int j) {return b + i*Domain_size + j;}
  inline int X3(int b, int i, int j, int k) {return b + i*Domain_size*Domain_size + j*Domain_size + k;}
  inline int X4(int b, int i, int j, int k, int l) {return b + i*Domain_size*Domain_size*Domain_size + j*Domain_size*Domain_size + k*Domain_size + l;}

private:
  /* search return codes */
  enum {
    SEARCH_GO_MODELS,           /* continue: model(s) found on current path */
    SEARCH_GO_NO_MODELS,        /* continue: no models found on current path */
    SEARCH_MAX_MODELS,          /* stop */
    SEARCH_MAX_MEGS,            /* stop */
    SEARCH_MAX_TOTAL_SECONDS,   /* stop */
    SEARCH_MAX_DOMAIN_SECONDS,  /* stop */
    SEARCH_DOMAIN_OUT_OF_RANGE  /* stop */
  };

  static long long next_message;    //TODO: [choiwah] take care of this to make it thread-safe
  static int Next_report;     		//TODO: [choiwah] take care of this too

  /* stats for entire run */
  unsigned long long Total_models;      //TODO: [choiwah] take care of this - may not be meaningful for multi-threading

private:
  std::string max_models_str;
  std::string all_models_str;
  std::string exhausted_str;
  std::string max_megs_yes_str;
  std::string max_megs_no_str;
  std::string max_sec_yes_str;
  std::string max_sec_no_str;
  std::string mace_sigint_str;
  std::string mace_sigsegv_str;
  std::string unknown_str;

  /*
   * The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object owns all of them.  They are "pointers" passed to other objects,
   * so all other objects see the same "running environment".
   */

  int         Number_of_cells;
  Cell        Cells;             /* the table of cells (dynamically allocated) */
  Cell*       Ordered_cells;     /* (pointers to) permutation of Cells */
  int         First_skolem_cell;
  int         Domain_size;       /* domain size to search */
  int         print_cubes;		 // print the cubes of length "print_cubes". -1 means do not print
  int         cubes_options;     // 0 nothing, bit 1-use work stealing, bit 2-  bit 3 -
  Term*       Domain;            /* array of terms representing (shared) domain elements  */
  bool        Skolems_last;
  Plist       Models;

  int         Max_domain_element_in_input;  /* For Least Number Heuristic */

  Symbol_data    Symbols;
  Symbol_data*   Sn_to_mace_sn;
  int            Sn_map_size;
  propagate*     propagator;
  myClock        mace4_clock;
  struct mace_stats   Mstats;
  MstateContainer     MScon;
  EstackContainer     EScon;
  Ground*             Grounder;

  /* Cached symbol numbers */
  Mace4GlobalValues      mace4_gv;

  // Memory/Time stuff
  int    Start_megs;
  double Start_seconds;
  double Start_domain_seconds;

  // Global data for searching
  Mace4VGlobais* Mace4vglobais;
  std::vector<std::vector<int>>  all_nodes;

  // printing functions
  static std::string interp_file_name;
  ofstream* models_interp_file_stream = nullptr;
  size_t out_models_count = 0;
  size_t file_count = 1;
  int  id2val(int id);
  int  f0_val(int base);
  int  f1_val(int base, int i);
  int  f2_val(int base, int i, int j);
  void print_model_standard(std::ostream& os, bool print_head);
  void print_model_interp(std::ostream& os);
  void p_model(bool print_head);
  void p_matom(Term atom);
  int  eterms_count(Term t);
  void p_eterms(void);
  void p_stats(void);
  void p_mem(void);

  void initialize_for_search(Plist clauses);
  void init_for_domain_size(void);
  void built_in_assignments(void);
  void special_assignments(void);
  int  possible_model(void);
  Term interp_term(void);
  int  mace_megs(void);
  int  current_time(void) const { return (int) myClock::user_seconds(); };
  int  check_time_memory(int seconds);
  bool mace4_skolem_check(int id);
  int  search(int max_constrained, int depth, Cube& splitter);
  int  mace4n(Plist clauses, int order);
  bool iterate_ok(int n, const std::string& class_name);
  int  next_domain_size(int n);

public:
  Search() = delete;
  ~Search() = default;   // memory are freed on exit of each search

  Search(Mace4VGlobais* g);

  Mace_results mace4(Plist clauses);

public:
  friend class MACE4;
};

#endif


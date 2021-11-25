#ifndef _SEARCH_H
#define _SEARCH_H

#include "./ladr/clock.h"
#include "./ladr/top_input.h"
#include "pred_elim.h"
#include "index_lits.h"
#include "demodulate.h"

#include "giv_select.h"
#include "actions.h"

#include "search_structures.h"
#include "unistd.h"
#include "semantics.h"

#include <iostream>
#include <fstream>
#include <limits>


#define DBL_LARGE   std::numeric_limits<double>::max()

#define MAX_HSIZE 5000

using namespace std;





 struct TGlob {

  // basic clause lists

  Clist sos;
  Clist usable;
  Clist demods;
  Clist hints;

  // other lists

  Plist actions;
  Plist weights;
  Plist kbo_weights;
  Plist interps;
  Plist given_selection;
  Plist keep_rules;
  Plist delete_rules;

  // auxiliary clause lists

  Clist limbo;
  Clist disabled;
  Plist empties;

  // indexing

  Lindex clashable_idx;  // literal index for resolution rules
  bool use_clash_idx;    // GET RID OF THIS VARIABLE!!

  // basic properties of usable+sos

  bool horn, unit, equality ;
  int number_of_clauses, number_of_neg_clauses;

  // other stuff

  Plist desc_to_be_disabled;   // Descendents of these to be disabled
  Plist cac_clauses;           // Clauses that trigger back CAC check

  bool searching;      // set to TRUE when first given is selected
  bool initialized;    // has this structure been initialized?
  double start_time;   // when was it initialized? 
  int start_ticks;     // quasi-clock that times the same for all machines

  int return_code;     // result of search
};


class GlobalSearch {
    
private:
                            jmp_buf Jump_env;                 // for setjmp/longjmp
                            Prover_options Opt;               // Prover9 options
                            prover_attributes Att;     // Prover9 accepted attributes
                            prover_stats Stats;        // Prover9 statistics
                            prover_clocks Clocks;      // Prover9 clocks
                            TGlob Glob;
                            int HIT_LIST[MAX_HSIZE];
                            int hsize;
                            
                            
                  public:   void Free_Mem(void);
                            
                            
                            friend class Search;
                            friend class Prover9VGlobais;
            
};


class Search {

private:
         
            static GivSelect GS;
            static JustContainer J;
            static UnifyContainer U;
            static myClock CLOCK;
            


    
            static void rebuild_sos_index(void);
            static void done_with_search(int);
            static void infer_outside_loop(Topform);
            static void cl_process_simplify(Topform);
            static void cl_process(Topform);
            static void exit_if_over_limit(void);

          
            static void fatal_setjmp(void);
            static Prover_results collect_prover_results(bool);
            static void update_stats(void);
            static void init_search(void);
            static void possible_report(void);
            static void cl_process_conflict(Topform, bool);
            static void cl_process_keep(Topform);
            static bool cl_process_delete(Topform);
            static void clause_wt_with_adjustments(Topform);
            static bool skip_black_white_tests(Topform);
            static bool restricted_denial(Topform);
            static void cl_process_new_demod(Topform);
            static void handle_proof_and_maybe_exit(Topform);
            static void limbo_process(bool);
            static void disable_clause(Topform);
            static void index_clashable(Topform, Indexop);
            static void back_demod(Topform);
            static void back_unit_deletion(Topform);
            static void back_cac_simplify(void);
            static void disable_to_be_disabled(void);
            static void basic_clause_properties(Clist, Clist);
            static void auto_denials(Clist, Clist, Prover_options);
            static void index_and_process_initial_clauses(void);
            static Topform orient_input_eq(Topform);
            static bool inferences_to_make(void);
            static void make_inferences(void); 
            static void given_infer(Topform);
            static void fprint_prover_stats(ostream &, struct prover_stats, string); 
            static void fprint_prover_clocks(ostream &, struct prover_clocks);
            static void auto_process(Clist, Clist, Prover_options);
            static void auto_inference(Clist, Clist, Prover_options);
            static int degradation_count(Topform);
            static void print_derivation(Topform);
            static void hint_derivation(Topform);
            static void get_hit_list(void);
            static bool  on_hit_list(int);

public:

    
            static Prover_input input;
            static Prover_results results;    
                
            static Prover_results search(Prover_input);
            static int exit_with_message(ostream &, int);  
            static string exit_string(int code);
            static Prover_options init_prover_options(void);
            static void init_prover_attributes(void);
            static void fprint_all_stats(ostream &, string);
            static void free_search_memory(void);
            static void destroy_prover_input(Prover_input);
            static void report(ostream &, string);
            static void prochints(Prover_input);

        
    
};


#endif

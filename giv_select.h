#ifndef TP_GIV_SELECT_H
#define TP_GIV_SELECT_H



#include "./ladr/clause_eval.h"
#include "./ladr/banner.h"
#include "./ladr/avltree.h"
#include "./ladr/clist.h"
#include "search_structures.h"


enum class GsOrder { GS_ORDER_WEIGHT,
					 GS_ORDER_AGE,
                     // BV(2016-may-27)
                     GS_ORDER_HINT_AGE,
					 GS_ORDER_RANDOM
}; 


typedef struct giv_select *Giv_select;

struct giv_select {
  string       *name;
  int          order;
  Clause_eval  property;
  int          part;
  int          selected;
  OrderType (*compare) (void *, void *);  /* function for ordering idx */
  Avl_node idx;          /* index of clauses (binary search (AVL) tree) */
};



typedef struct select_state *Select_state;

/* Static variables */

 struct select_state{
  Plist selectors;    /* list of Giv_select */
  int occurrences;    /* occurrences of clauses in selectors */
  Plist current;      /* for ratio state */
  int  count;         /* for ratio state */
  int  cycle_size;
};


class GlobalGivSelect {

                        private:  	
                                select_state High, Low;
								bool Rule_needs_semantics;
								int Sos_size;
								double Low_water_keep;
								double Low_water_displace;
								int Sos_deleted;
								int Sos_displaced;
								bool Debug;
								unsigned Giv_select_gets; 
								unsigned Giv_select_frees; 
                                
                                void zap_given_selectors(void);
                            
                        public:   
                            
                                void Free_Mem(void);
                                
                        friend class GivSelect;
                        friend class Prover9VGlobais;
    
};



class GivSelect {


					private:
							
                                AvltreeContainer AVL;
                                TermContainer T;
                                PlistContainer P;
                                ClauseEvalContainer CEVAL;
                                TopformContainer TF;
                                ClistContainer CL;
					
								Giv_select get_giv_select(void);
								void free_giv_select(Giv_select);
								int current_cycle_size(Select_state);
								void update_selectors(Topform, bool insert);
								Giv_select next_selector(Select_state);
								double iterations_to_selection(int,int,int, int, int);
								double least_iters_to_selection(Topform, Select_state, Plist );
                                Topform worst_clause_of_priority_group(Select_state);
                                Topform worst_clause(void);

					
					

					public:
							void init_giv_select(Plist);
							void insert_into_sos2(Topform, Clist);
							void remove_from_sos2(Topform, Clist);
							bool givens_available(void);
							Topform get_given_clause2(Clist, int,Prover_options, string &);
							bool sos_keep2(Topform, Clist, Prover_options);
							void sos_displace2(void (*disable_proc) (Topform));
							
							void selector_report(void);
							Term selector_rule_term(const string &, const string &,const string &,const string &, int);
							Plist selector_rules_from_options(Prover_options);
            
                            friend class GlobalGivSelect;
                            friend class Search;
};


#endif

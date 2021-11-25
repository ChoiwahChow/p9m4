#ifndef TP_UTILITIES_H
#define TP_UTILITIES_H


#include "./ladr/clash.h"
#include "./ladr/ioutil.h"
#include "search_structures.h"

class Utilities {
					private:
					
					static void fsym_collect(Ilist *, Term,int);
					static bool inverse_axiom(Topform, int *, int *, int *);
					static int neg_pos_depth_diff(Topform);
					static int neg_pos_wt_diff(Topform);
					static void p_term_list(Plist);
					static Topform next_negative_clause_3(Clist_pos *, Clist_pos *, Clist_pos *);

	
					public:
					
					static void print_memory_stats(ostream &);
					static void fsym_report(Ilist, Plist);
					static bool inverse_order(Clist);
					static void p_sym_list(Ilist);
					static void symbol_order(Clist, Clist, Clist, bool);
                    
                    
                    
					static Ilist unary_symbols(Ilist);
					static void auto_kbo_weights(Clist, Clist);
					static int neg_pos_depth_difference(Plist);
					static void structure_of_clauses(Clist);
					static int plist_size_of_diff(Plist, Plist);
					static void structure_of_variables(Clist);
					static OrderType clause_compare_m4(Topform, Topform);
					static int bogo_ticks(void);
					static Topform first_negative_clause(Plist);
					static Plist neg_clauses_and_descendants(Plist,Clist, Clist, Clist);
					static Plist neg_descendants(Topform,Clist,Clist,Clist);
					static void check_constant_sharing(Plist);
};

#endif

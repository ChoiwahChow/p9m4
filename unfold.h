#ifndef TP_UNFOLD_H
#define TP_UNFOLD_H

#include "./ladr/parautil.h"
#include "./ladr/clist.h"
#include "./ladr/ioutil.h"


enum class UnfoldCheckType{
						NOT_CHECKED = 0, 
						CHECKING, 
						CHECKED, 
						CYCLE
					};


typedef struct {
    				Term alpha, beta;
					Ilist dependents;
                    int status;
	
}Tsymdata;

                    
class Unfold {

				private:
                    static Tsymdata symdata;
				
								
					static void trace_dependents(int, Tsymdata *);	
					static Ilist eliminate_cycles(Ilist symbols, Tsymdata *);
					static bool sym_less_or_equal(int, int, Tsymdata *);
					static OrderType compare_symbols(int, int, Tsymdata *);
					static Ilist insert_symbol(Ilist, int, Tsymdata *);
					static Ilist order_symbols(Ilist, Tsymdata *);
					static Ilist eq_defs(Clist, int);
					static int num_constant_symbols(Ilist);
					static bool constant_check(int, Ilist, Clist, int);
					static Ilist remove_kb_wt_zero(Ilist);
			

				public:
				
					static void unfold_eq_defs(Clist,int,int,bool);
					static bool fold_eq_defs(Clist,bool);
					static bool one_unary_def(Clist, Clist);

};


#endif

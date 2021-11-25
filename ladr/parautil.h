#ifndef TP_PARAUTIL_H
#define TP_PARAUTIL_H

#include "just.h"


class GlobalParautil {
						private:
						
								int Oriented_flag;       /* termflag to mark oriented equality atoms */
								int Renamable_flip_flag; /* termflag to mark renamable-flip eq atoms */
	
                        public:
                                GlobalParautil();
                                ~GlobalParautil();
                                
                                
						friend class Parautil;
                        friend class LadrVGlobais;
};


class Parautil {
						private:
								Topform clause_with_new_constant(Topform, Term, int);
						
						
						public:
						
								void init_paramod(void);
								void mark_renamable_flip(Term);
								bool renamable_flip_eq(Term);
								bool renamable_flip_eq_test(Term);
								void mark_oriented_eq(Term);
								bool oriented_eq(Term);
								bool same_term_structure(Term, Term);
								void flip_eq(Term, int);
								void orient_equalities(Topform, bool allow_flips);
								bool eq_tautology(Topform);
								Term top_flip(Term);
								void zap_top_flip(Term);
								Literals literal_flip(Literals);
								void zap_literal_flip(Literals);
								Topform new_constant(Topform, int);
								Topform fold_denial(Topform, int);
								bool equational_def_2(Term, Term);
								int equational_def(Topform);
								OrderType unfold_order(Term, Term);
								Topform build_reflex_eq(void);

	
};


#endif

 
 

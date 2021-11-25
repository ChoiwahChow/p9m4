#ifndef TP_CNF_H
#define TP_CNF_H

#include "formula.h"
#include "clock.h"
#include <setjmp.h>  
#include <signal.h>
#include <unistd.h>

typedef void (*sighandler_t)(int);


class GlobalCnf {
					private:
								jmp_buf Jump_env;        /* for setjmp/longjmp */
								unsigned Fid_call_limit;
								unsigned Fid_calls;
								
                    public:			
                                GlobalCnf();
                                ~GlobalCnf();
                        
                                friend class Cnf;
                                friend class LadrVglobais;
	
};



class Cnf {
				private:
						Formula share_formula(Formula, Hashtab);
						Formula consolidate_formula(Formula);
						bool complementary_share(Formula, Formula);
						bool contains_complements_share(Formula);
						bool prop_member_share(Formula, Formula);
						bool prop_subset_share(Formula, Formula);
						bool prop_subsume_share(Formula, Formula);
						Formula remove_subsumed_share(Formula);
						Formula bbt(Formula, int, int); 
						Formula balanced_binary(Formula);
						Formula disjoin_flatten_simplify(Formula, Formula);
						Formula simplify_and_share(Formula);
						Formula distribute_top(Formula);
						Formula distribute(Formula);
                        static bool formula_ident(void *, void *);
                        
                     
                        Formula skolem(Formula, Ilist);
                        Ilist unique_qvars(Formula, Ilist);
                        void mark_free_vars_formula(Formula , string, int); 
                        Formula remove_uni_quant(Formula, int *);
                        Formula ms_free_vars(Formula);
                      
						
						
				public:
						bool formula_ident_share(Formula, Formula);
						Formula formula_copy_share(Formula);
						Formula cnf(Formula);
						Formula dnf(Formula);
						Formula skolemize(Formula);
						Formula unique_quantified_vars(Formula);
						Formula remove_universal_quantifiers(Formula);
						Formula clausify_prepare(Formula);
						Formula miniscope(Formula);
						Formula miniscope_formula(Formula, unsigned mega_fid_call_limit);
						int cnf_max_clauses(Formula);
	
	
};

#endif

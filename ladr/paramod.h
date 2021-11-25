#ifndef TP_PARAMOD_H
#define TP_PARAMOD_H

#include "resolve.h"
#include "basic.h"


enum class Para_loc { 
			PARA_ALL,
			PARA_ALL_EXCEPT_TOP,
			PARA_TOP_ONLY 
			};
			
class Paramodulation {


			private:
					static bool  Ordered_inference;
					static bool  Positive_inference;
					static bool  Para_from_vars;
					static bool  Para_into_vars;
					static bool  Para_from_small;
					static bool  Check_instances;  /* non-oriented from lits */

					static int Para_instance_prunes;     /* counter */
					static int Basic_prunes;  
					
					
					static bool basic_check(Term);
					static Literals apply_lit_para(Literals, Context);
					static bool para_from_right(Term);
					static bool from_parent_test(Literals, int);
					static bool into_parent_test(Literals, int);
					static bool check_instance(Literals, Context, bool);
					static bool check_instances(Literals, int, Context, Literals, Term, Context);
					static void para_into(Literals from_lit, int from_side, Context cf, Ilist from_pos,
											Topform into_clause, Literals into_lit, Term into, Context ci,
											Ilist into_pos,
											bool skip_top,
											void (*proc_proc) (Topform));
											
					static void para_into_lit(Literals from_lit, int from_side, Context cf,
												Literals into_lit, Context ci,
												bool check_top,
												void (*proc_proc) (Topform));
												
					#if 0
						static Term apply_substitute_para(Term, Term, Context,Term, Context);
					#endif
	
			public:
					static void paramodulation_options(bool,bool,bool,bool,bool,bool,bool);
					static int para_instance_prunes();
					static int basic_paramodulation_prunes(void);
					static Topform paramodulate(Literals, int, Context,Topform,Ilist,Context);
					static void para_from_into(Topform from, Context cf, Topform into, Context ci,bool check_top, void (*proc_proc) (Topform));
					static Topform para_pos(Topform, Ilist,Topform, Ilist);
					static Topform para_pos2(Topform,Ilist,Topform,Ilist);
};			


#endif
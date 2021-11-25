#ifndef TP_RESOLVE_H
#define TP_RESOLVE_H

#include "clash.h"
#include "lindex.h"
#include "ladrvglobais.h"


enum class Resolve_lit_selection{  /* literal selection */
			
			LIT_SELECTION_NONE,
			LIT_SELECTION_MAXIMAL,
			LIT_SELECTION_ALL
};

enum class Resolve_type{  /* types of resolution (binary, hyper, UR) */
	POS_RES,  /* positive */
	NEG_RES,  /* negative */
	ANY_RES   /* unrestricted by sign */
};


class Resolve {
	
				private:
				
						static bool  Ordered            ;
						static bool  Check_instances    ;
						static int   Ur_nucleus_limit   ;  /* limit num of clashable lits */
						static bool  Initial_nuclei     ;    /* nuclei must be input clauses  */
						static bool  Production_mode    ;
						static int   Res_instance_prunes;
                        
                        static Parautil Pu;
                        static UnifyContainer U;
                        static JustContainer J;
                        static TermContainer T;
                        static MindexContainer M;
                        static ClashContainer C;
                        static AttributeContainer AT;
                        static TopformContainer TF;
                        static IlistContainer IL;
				
				
				
						static bool unit_check(Literals);
						static bool pos_hyper_sat_test(Literals);
						static bool neg_hyper_sat_test(Literals);
						static void hyper_sat_atom(bool flipped, Literals slit, Term atom, int pos_or_neg,Lindex idx, void (*proc_proc) (Topform));
						static void hyper_satellite(Topform c, int pos_or_neg, Lindex idx,void (*proc_proc) (Topform));
						static void hyper_nucleus(Topform c, int pos_or_neg, Lindex idx, void (*proc_proc) (Topform));
						static bool target_check(Literals, int);
						static void ur_sat_atom(bool flipped, Topform c, int target_constraint, Term sat_atom, Lindex idx, void (*proc_proc) (Topform));
						static void ur_satellite(Topform c, int target_constraint, Lindex idx,void (*proc_proc) (Topform));
						static void ur_nucleus(Topform c, int target_constraint, Lindex idx,void (*proc_proc) (Topform));
						static void xx_res(Literals lit, void (*proc_proc) (Topform));
						static void binary_resolvent(bool flipped, Literals l1, Context s1, Literals l2, Context s2, void (*proc_proc) (Topform));
						static bool binary_parent_test(Literals, int , int);
						static bool check_instance(Literals, Context, int);
						static bool check_instances(Literals, Context, Literals, Context, int);
						static void bin_res_lit(Topform giv, Literals lit, Term atom, int res_type, Lindex idx, void (*proc_proc) (Topform));
						
				
				public:
				
						static void resolution_options(bool,bool,bool, int,bool);
						static int res_instance_prunes();
						static void hyper_resolution(Topform c, int pos_or_neg, Lindex idx, void (*proc_proc) (Topform));
						static void ur_resolution(Topform c, int target_constraint, Lindex idx, void (*proc_proc) (Topform));
						static Topform instantiate_clause(Topform, Context);
						static void binary_resolution(Topform c,int res_type,  /* POS_RES, NEG_RES, ANY_RES */Lindex idx,void (*proc_proc) (Topform));
						static void binary_factors(Topform c, void (*proc_proc) (Topform));
						static void merge_literals(Topform);
						static Topform copy_inference(Topform);
						static Topform resolve2(Topform, int, Topform, int, bool);
						static Topform resolve3(Topform, Literals, Topform, Literals, bool);
						static Topform xx_resolve2(Topform, int, bool);
	
};


#endif
